#include "Domain.h"

#include <algorithm>
#include <stdexcept>

namespace cbt {

std::string toString(EventType type) {
    return type == EventType::Lecture ? "Lecture" : "Lab";
}

EventType eventTypeFromString(const std::string& value) {
    if (value == "Lab" || value == "lab" || value == "LAB") {
        return EventType::Lab;
    }
    return EventType::Lecture;
}

void ProblemData::buildIndexes() {
    courseById.clear();
    instructorById.clear();
    roomById.clear();
    groupById.clear();
    timeSlotById.clear();
    blockedCourseTimeSlots.clear();
    explicitCourseConflicts.clear();

    for (const auto& item : courses) {
        courseById[item.id] = item;
    }
    for (const auto& item : instructors) {
        instructorById[item.id] = item;
    }
    for (const auto& item : rooms) {
        roomById[item.id] = item;
    }
    for (const auto& item : studentGroups) {
        groupById[item.id] = item;
    }
    for (const auto& item : timeSlots) {
        timeSlotById[item.id] = item;
    }

    for (const auto& record : constraints) {
        if (record.type == "BLOCKED_TIMESLOT") {
            blockedCourseTimeSlots[record.primary].insert(record.secondary);
        } else if (record.type == "EXPLICIT_CONFLICT") {
            auto left = std::min(record.primary, record.secondary);
            auto right = std::max(record.primary, record.secondary);
            explicitCourseConflicts.insert({left, right});
        } else if (record.type == "PREFERRED_TIMESLOT") {
            instructorById[record.primary].preferredTimeSlots.insert(record.secondary);
        } else if (record.type == "UNAVAILABLE_TIMESLOT") {
            instructorById[record.primary].availableTimeSlots.erase(record.secondary);
        }
    }
}

const Course& ProblemData::course(const std::string& id) const {
    return courseById.at(id);
}

const Instructor& ProblemData::instructor(const std::string& id) const {
    return instructorById.at(id);
}

const Room& ProblemData::room(const std::string& id) const {
    return roomById.at(id);
}

const StudentGroup& ProblemData::group(const std::string& id) const {
    return groupById.at(id);
}

const TimeSlot& ProblemData::timeSlot(const std::string& id) const {
    return timeSlotById.at(id);
}

} // namespace cbt
