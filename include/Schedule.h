#pragma once

#include "Domain.h"

#include <optional>
#include <vector>

namespace cbt {

class Schedule {
public:
    void add(const Assignment& assignment);
    void removeEvent(const std::string& eventId);
    bool containsEvent(const std::string& eventId) const;
    std::optional<Assignment> assignmentForEvent(const std::string& eventId) const;
    const std::vector<Assignment>& assignments() const;
    std::vector<Assignment> byInstructor(const std::string& instructorId) const;
    std::vector<Assignment> byStudentGroup(const std::string& groupId) const;
    std::vector<Assignment> byRoom(const std::string& roomId) const;
    void clear();
    std::size_t size() const;

private:
    std::vector<Assignment> assignments_;
};

} // namespace cbt
