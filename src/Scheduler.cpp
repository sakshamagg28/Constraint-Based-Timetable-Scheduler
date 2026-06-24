#include "Scheduler.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <sstream>

namespace cbt {

Scheduler::Scheduler(ProblemData data) : data_(std::move(data)) {
    data_.buildIndexes();
    graph_.build(data_);
    createEvents();
    createDomains();
}

ScheduleResult Scheduler::solve() {
    const auto start = std::chrono::steady_clock::now();
    ScheduleResult result;

    for (const auto& [eventId, domain] : domains_) {
        if (domain.empty()) {
            diagnostics_.push_back("Event " + eventId + " has an empty domain before search.");
        }
    }

    if (diagnostics_.empty()) {
        result.success = backtrack();
    }

    const auto end = std::chrono::steady_clock::now();
    metrics_.runtimeSeconds = std::chrono::duration<double>(end - start).count();

    if (result.success) {
        Validator validator(data_);
        auto validation = validator.validateComplete(working_, events_);
        result.success = validation.valid;
        if (!validation.valid) {
            diagnostics_.insert(diagnostics_.end(), validation.errors.begin(), validation.errors.end());
        }
    }

    result.schedule = working_;
    QualityScorer scorer(data_);
    result.quality = scorer.score(result.schedule);
    metrics_.finalScore = result.quality.score;
    result.metrics = metrics_;
    result.diagnostics = diagnostics_;
    return result;
}

const ConflictGraph& Scheduler::conflictGraph() const {
    return graph_;
}

void Scheduler::createEvents() {
    events_.clear();
    for (const auto& course : data_.courses) {
        for (int i = 1; i <= course.weeklyLectures; ++i) {
            events_.push_back({course.id + "_LEC_" + std::to_string(i), course.id, EventType::Lecture, i});
        }
        for (int i = 1; i <= course.weeklyLabs; ++i) {
            events_.push_back({course.id + "_LAB_" + std::to_string(i), course.id, EventType::Lab, i});
        }
    }
}

void Scheduler::createDomains() {
    domains_.clear();
    for (const auto& event : events_) {
        const auto& course = data_.course(event.courseId);
        const std::string roomType = requiredRoomType(event);
        auto& domain = domains_[event.id];

        for (const auto& instructorId : course.eligibleInstructors) {
            const auto& instructor = data_.instructor(instructorId);
            for (const auto& room : data_.rooms) {
                if (room.type != roomType || room.capacity < course.enrolledStudents) {
                    continue;
                }
                for (const auto& slot : data_.timeSlots) {
                    if (instructor.availableTimeSlots.count(slot.id) == 0) {
                        continue;
                    }
                    auto blockedIt = data_.blockedCourseTimeSlots.find(course.id);
                    if (blockedIt != data_.blockedCourseTimeSlots.end() && blockedIt->second.count(slot.id) > 0) {
                        continue;
                    }
                    domain.push_back({instructorId, room.id, slot.id});
                }
            }
        }
    }
}

std::string Scheduler::requiredRoomType(const CourseEvent& event) const {
    return event.type == EventType::Lab ? "Lab" : data_.course(event.courseId).roomType;
}

Assignment Scheduler::makeAssignment(const CourseEvent& event, const DomainValue& value) const {
    const auto& course = data_.course(event.courseId);
    const auto& instructor = data_.instructor(value.instructorId);
    const auto& room = data_.room(value.roomId);
    const auto& slot = data_.timeSlot(value.timeSlotId);
    const auto& group = data_.group(course.studentGroupId);

    Assignment assignment;
    assignment.eventId = event.id;
    assignment.courseId = course.id;
    assignment.courseName = course.name;
    assignment.eventType = event.type;
    assignment.instructorId = instructor.id;
    assignment.instructorName = instructor.name;
    assignment.roomId = room.id;
    assignment.roomName = room.name;
    assignment.timeSlotId = slot.id;
    assignment.day = slot.day;
    assignment.start = slot.start;
    assignment.end = slot.end;
    assignment.studentGroupId = group.id;
    assignment.studentGroupName = group.name;
    assignment.enrolledStudents = course.enrolledStudents;
    return assignment;
}

bool Scheduler::backtrack() {
    ++metrics_.searchNodesVisited;
    if (working_.size() == events_.size()) {
        return true;
    }

    auto next = selectUnassignedEvent();
    if (!next) {
        return false;
    }

    auto values = legalValues(*next);
    if (values.empty()) {
        ++metrics_.deadEnds;
        diagnostics_.push_back("Dead end: no legal values remain for " + next->id + ".");
        return false;
    }

    struct RankedValue {
        DomainValue value;
        int flexibility = 0;
        int preferencePenalty = 0;
    };

    std::vector<RankedValue> ranked;
    ranked.reserve(values.size());
    for (const auto& value : values) {
        ranked.push_back({value, remainingValueCountAfter(*next, value), localPreferencePenalty(*next, value)});
    }
    std::sort(ranked.begin(), ranked.end(), [](const RankedValue& a, const RankedValue& b) {
        if (a.flexibility != b.flexibility) {
            return a.flexibility > b.flexibility;
        }
        return a.preferencePenalty < b.preferencePenalty;
    });

    Validator validator(data_);
    for (const auto& rankedValue : ranked) {
        const auto& value = rankedValue.value;
        Assignment assignment = makeAssignment(*next, value);
        ++metrics_.assignmentsAttempted;
        if (!validator.canPlace(working_, assignment, &metrics_)) {
            continue;
        }
        working_.add(assignment);
        if (forwardCheck() && backtrack()) {
            return true;
        }
        working_.removeEvent(assignment.eventId);
        ++metrics_.backtracks;
    }

    ++metrics_.deadEnds;
    return false;
}

std::optional<CourseEvent> Scheduler::selectUnassignedEvent() {
    std::optional<CourseEvent> best;
    int bestRemaining = std::numeric_limits<int>::max();
    int bestDegree = -1;

    for (const auto& event : events_) {
        if (working_.containsEvent(event.id)) {
            continue;
        }
        int remaining = static_cast<int>(legalValues(event).size());
        int degree = graph_.degree(event.courseId);
        if (!best || remaining < bestRemaining || (remaining == bestRemaining && degree > bestDegree)) {
            best = event;
            bestRemaining = remaining;
            bestDegree = degree;
        }
    }
    return best;
}

std::vector<Scheduler::DomainValue> Scheduler::legalValues(const CourseEvent& event) {
    std::vector<DomainValue> values;
    Validator validator(data_);
    const auto& domain = domains_.at(event.id);
    for (const auto& value : domain) {
        if (validator.canPlace(working_, makeAssignment(event, value), &metrics_)) {
            values.push_back(value);
        }
    }
    return values;
}

int Scheduler::remainingValueCountAfter(const CourseEvent& event, const DomainValue& value) {
    Assignment assignment = makeAssignment(event, value);
    Validator validator(data_);
    if (!validator.canPlace(working_, assignment, &metrics_)) {
        return -1;
    }

    working_.add(assignment);
    int remaining = 0;
    for (const auto& other : events_) {
        if (working_.containsEvent(other.id)) {
            continue;
        }
        if (!sameCourseOrConflict(event, other)) {
            continue;
        }
        const auto& domain = domains_.at(other.id);
        for (const auto& futureValue : domain) {
            const bool sameTime = futureValue.timeSlotId == value.timeSlotId;
            const bool sameInstructor = futureValue.instructorId == value.instructorId;
            const bool sameRoom = futureValue.roomId == value.roomId;
            if (sameTime && (sameInstructor || sameRoom || graph_.hasEdge(event.courseId, other.courseId))) {
                continue;
            }
            ++remaining;
        }
    }
    working_.removeEvent(event.id);
    return remaining;
}

int Scheduler::localPreferencePenalty(const CourseEvent& event, const DomainValue& value) const {
    int penalty = 0;
    const auto& slot = data_.timeSlot(value.timeSlotId);
    const auto& instructor = data_.instructor(value.instructorId);
    if (slot.late) {
        penalty += 10;
    }
    if (instructor.preferredTimeSlots.count(slot.id) == 0) {
        penalty += 3;
    }
    for (const auto& assignment : working_.assignments()) {
        if (assignment.courseId == event.courseId && assignment.day == slot.day) {
            penalty += 4;
        }
    }
    return penalty;
}

bool Scheduler::forwardCheck() {
    for (const auto& event : events_) {
        if (working_.containsEvent(event.id)) {
            continue;
        }
        if (legalValues(event).empty()) {
            return false;
        }
    }
    return true;
}

bool Scheduler::sameCourseOrConflict(const CourseEvent& left, const CourseEvent& right) const {
    return left.courseId == right.courseId || graph_.hasEdge(left.courseId, right.courseId);
}

} // namespace cbt
