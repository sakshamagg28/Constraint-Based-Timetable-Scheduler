#include "QualityScorer.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <sstream>

namespace cbt {
namespace {

using DaySlots = std::map<int, std::vector<Assignment>>;

int idleGaps(const std::vector<Assignment>& assignments, const ProblemData& data) {
    DaySlots byDay;
    for (const auto& assignment : assignments) {
        const auto& slot = data.timeSlot(assignment.timeSlotId);
        byDay[slot.dayIndex].push_back(assignment);
    }

    int gaps = 0;
    for (auto& [_, dayAssignments] : byDay) {
        std::vector<int> slots;
        for (const auto& assignment : dayAssignments) {
            slots.push_back(data.timeSlot(assignment.timeSlotId).slotIndex);
        }
        std::sort(slots.begin(), slots.end());
        slots.erase(std::unique(slots.begin(), slots.end()), slots.end());
        for (std::size_t i = 1; i < slots.size(); ++i) {
            gaps += std::max(0, slots[i] - slots[i - 1] - 1);
        }
    }
    return gaps;
}

int roomSwitches(const std::vector<Assignment>& assignments, const ProblemData& data) {
    DaySlots byDay;
    for (const auto& assignment : assignments) {
        byDay[data.timeSlot(assignment.timeSlotId).dayIndex].push_back(assignment);
    }

    int switches = 0;
    for (auto& [_, dayAssignments] : byDay) {
        std::sort(dayAssignments.begin(), dayAssignments.end(), [&](const Assignment& a, const Assignment& b) {
            return data.timeSlot(a.timeSlotId).slotIndex < data.timeSlot(b.timeSlotId).slotIndex;
        });
        for (std::size_t i = 1; i < dayAssignments.size(); ++i) {
            const int previous = data.timeSlot(dayAssignments[i - 1].timeSlotId).slotIndex;
            const int current = data.timeSlot(dayAssignments[i].timeSlotId).slotIndex;
            if (current == previous + 1 && dayAssignments[i - 1].roomId != dayAssignments[i].roomId) {
                ++switches;
            }
        }
    }
    return switches;
}

int isolatedSingleClasses(const std::vector<Assignment>& assignments, const ProblemData& data) {
    DaySlots byDay;
    for (const auto& assignment : assignments) {
        byDay[data.timeSlot(assignment.timeSlotId).dayIndex].push_back(assignment);
    }
    int isolated = 0;
    for (const auto& [_, dayAssignments] : byDay) {
        if (dayAssignments.size() == 1) {
            ++isolated;
        }
    }
    return isolated;
}

} // namespace

QualityScorer::QualityScorer(const ProblemData& data) : data_(data) {}

QualityBreakdown QualityScorer::score(const Schedule& schedule) const {
    QualityBreakdown out;
    int instructorGaps = 0;
    int studentGaps = 0;
    int lateSlots = 0;
    int instructorSwitches = 0;
    int studentSwitches = 0;
    int isolated = 0;
    int preferredMisses = 0;
    int repeatedCourseDays = 0;

    for (const auto& instructor : data_.instructors) {
        auto assignments = schedule.byInstructor(instructor.id);
        instructorGaps += idleGaps(assignments, data_);
        instructorSwitches += roomSwitches(assignments, data_);
        isolated += isolatedSingleClasses(assignments, data_);
    }

    for (const auto& group : data_.studentGroups) {
        auto assignments = schedule.byStudentGroup(group.id);
        studentGaps += idleGaps(assignments, data_);
        studentSwitches += roomSwitches(assignments, data_);
        isolated += isolatedSingleClasses(assignments, data_);
    }

    std::map<std::string, int> roomUse;
    std::map<std::string, std::map<int, int>> courseDayUse;
    for (const auto& room : data_.rooms) {
        roomUse[room.id] = 0;
    }

    for (const auto& assignment : schedule.assignments()) {
        const auto& slot = data_.timeSlot(assignment.timeSlotId);
        if (slot.late) {
            ++lateSlots;
        }
        if (data_.instructor(assignment.instructorId).preferredTimeSlots.count(assignment.timeSlotId) == 0) {
            ++preferredMisses;
        }
        ++roomUse[assignment.roomId];
        ++courseDayUse[assignment.courseId][slot.dayIndex];
    }

    for (const auto& [_, byDay] : courseDayUse) {
        for (const auto& [__, count] : byDay) {
            if (count > 1) {
                repeatedCourseDays += count - 1;
            }
        }
    }

    double averageUse = 0.0;
    for (const auto& [_, count] : roomUse) {
        averageUse += count;
    }
    averageUse = roomUse.empty() ? 0.0 : averageUse / static_cast<double>(roomUse.size());
    double utilizationVariance = 0.0;
    for (const auto& [_, count] : roomUse) {
        utilizationVariance += (count - averageUse) * (count - averageUse);
    }
    utilizationVariance = roomUse.empty() ? 0.0 : utilizationVariance / static_cast<double>(roomUse.size());

    out.rawPenalty = instructorGaps * 5.0
                   + studentGaps * 6.0
                   + lateSlots * 8.0
                   + repeatedCourseDays * 4.0
                   + instructorSwitches * 2.0
                   + studentSwitches * 2.0
                   + std::sqrt(utilizationVariance) * 3.0
                   + isolated * 5.0
                   + preferredMisses * 3.0;
    out.score = std::max(0.0, 100.0 - out.rawPenalty / 10.0);

    out.notes.push_back("Instructor idle gaps: " + std::to_string(instructorGaps));
    out.notes.push_back("Student idle gaps: " + std::to_string(studentGaps));
    out.notes.push_back("Late slots: " + std::to_string(lateSlots));
    out.notes.push_back("Repeated same-course day placements: " + std::to_string(repeatedCourseDays));
    out.notes.push_back("Instructor room switches: " + std::to_string(instructorSwitches));
    out.notes.push_back("Student room switches: " + std::to_string(studentSwitches));
    out.notes.push_back("Preferred time misses: " + std::to_string(preferredMisses));
    return out;
}

} // namespace cbt
