#include "Validator.h"

#include <algorithm>
#include <sstream>
#include <unordered_map>

namespace cbt {

void ValidationResult::addError(const std::string& message) {
    valid = false;
    errors.push_back(message);
}

Validator::Validator(const ProblemData& data) : data_(data) {}

bool Validator::canPlace(const Schedule& schedule, const Assignment& candidate, SearchMetrics* metrics) const {
    if (metrics) {
        ++metrics->constraintChecks;
    }

    if (candidate.roomId.empty() || candidate.timeSlotId.empty()) {
        return false;
    }

    const auto& course = data_.course(candidate.courseId);
    const auto& instructor = data_.instructor(candidate.instructorId);
    const auto& room = data_.room(candidate.roomId);

    if (std::find(course.eligibleInstructors.begin(), course.eligibleInstructors.end(), candidate.instructorId)
        == course.eligibleInstructors.end()) {
        return false;
    }
    if (instructor.availableTimeSlots.count(candidate.timeSlotId) == 0) {
        return false;
    }
    auto blockedIt = data_.blockedCourseTimeSlots.find(candidate.courseId);
    if (blockedIt != data_.blockedCourseTimeSlots.end()
        && blockedIt->second.count(candidate.timeSlotId) > 0) {
        return false;
    }
    if (room.capacity < candidate.enrolledStudents) {
        return false;
    }
    const std::string requiredType = candidate.eventType == EventType::Lab ? "Lab" : course.roomType;
    if (room.type != requiredType) {
        return false;
    }

    for (const auto& existing : schedule.assignments()) {
        if (existing.eventId == candidate.eventId) {
            return false;
        }
        if (existing.timeSlotId != candidate.timeSlotId) {
            continue;
        }
        if (existing.instructorId == candidate.instructorId) {
            return false;
        }
        if (existing.roomId == candidate.roomId) {
            return false;
        }
        if (existing.studentGroupId == candidate.studentGroupId) {
            return false;
        }
        auto left = std::min(existing.courseId, candidate.courseId);
        auto right = std::max(existing.courseId, candidate.courseId);
        if (data_.explicitCourseConflicts.count({left, right}) > 0) {
            return false;
        }
    }

    return true;
}

ValidationResult Validator::validateComplete(const Schedule& schedule, const std::vector<CourseEvent>& events) const {
    ValidationResult result;
    std::unordered_map<std::string, int> lectureCounts;
    std::unordered_map<std::string, int> labCounts;

    if (schedule.size() != events.size()) {
        std::ostringstream os;
        os << "Schedule has " << schedule.size() << " assignments but requires " << events.size() << ".";
        result.addError(os.str());
    }

    for (const auto& event : events) {
        if (!schedule.containsEvent(event.id)) {
            result.addError("Missing assignment for event " + event.id + ".");
        }
    }

    Schedule prefix;
    for (const auto& assignment : schedule.assignments()) {
        if (assignment.eventType == EventType::Lecture) {
            ++lectureCounts[assignment.courseId];
        } else {
            ++labCounts[assignment.courseId];
        }
        if (!canPlace(prefix, assignment, nullptr)) {
            result.addError("Hard constraint violation detected at event " + assignment.eventId + ".");
        }
        prefix.add(assignment);
    }

    for (const auto& course : data_.courses) {
        if (lectureCounts[course.id] != course.weeklyLectures) {
            result.addError(course.name + " requires " + std::to_string(course.weeklyLectures)
                            + " lectures but has " + std::to_string(lectureCounts[course.id]) + ".");
        }
        if (labCounts[course.id] != course.weeklyLabs) {
            result.addError(course.name + " requires " + std::to_string(course.weeklyLabs)
                            + " labs but has " + std::to_string(labCounts[course.id]) + ".");
        }
    }

    return result;
}

} // namespace cbt
