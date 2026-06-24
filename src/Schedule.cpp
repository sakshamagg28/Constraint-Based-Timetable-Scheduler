#include "Schedule.h"

#include <algorithm>

namespace cbt {

void Schedule::add(const Assignment& assignment) {
    assignments_.push_back(assignment);
}

void Schedule::removeEvent(const std::string& eventId) {
    assignments_.erase(std::remove_if(assignments_.begin(), assignments_.end(),
                                      [&](const Assignment& item) { return item.eventId == eventId; }),
                       assignments_.end());
}

bool Schedule::containsEvent(const std::string& eventId) const {
    return std::any_of(assignments_.begin(), assignments_.end(),
                       [&](const Assignment& item) { return item.eventId == eventId; });
}

std::optional<Assignment> Schedule::assignmentForEvent(const std::string& eventId) const {
    auto it = std::find_if(assignments_.begin(), assignments_.end(),
                           [&](const Assignment& item) { return item.eventId == eventId; });
    if (it == assignments_.end()) {
        return std::nullopt;
    }
    return *it;
}

const std::vector<Assignment>& Schedule::assignments() const {
    return assignments_;
}

std::vector<Assignment> Schedule::byInstructor(const std::string& instructorId) const {
    std::vector<Assignment> out;
    std::copy_if(assignments_.begin(), assignments_.end(), std::back_inserter(out),
                 [&](const Assignment& item) { return item.instructorId == instructorId; });
    return out;
}

std::vector<Assignment> Schedule::byStudentGroup(const std::string& groupId) const {
    std::vector<Assignment> out;
    std::copy_if(assignments_.begin(), assignments_.end(), std::back_inserter(out),
                 [&](const Assignment& item) { return item.studentGroupId == groupId; });
    return out;
}

std::vector<Assignment> Schedule::byRoom(const std::string& roomId) const {
    std::vector<Assignment> out;
    std::copy_if(assignments_.begin(), assignments_.end(), std::back_inserter(out),
                 [&](const Assignment& item) { return item.roomId == roomId; });
    return out;
}

void Schedule::clear() {
    assignments_.clear();
}

std::size_t Schedule::size() const {
    return assignments_.size();
}

} // namespace cbt
