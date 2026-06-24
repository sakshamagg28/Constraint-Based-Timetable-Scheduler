#pragma once

#include "ConflictGraph.h"
#include "Domain.h"
#include "QualityScorer.h"
#include "Schedule.h"
#include "Validator.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace cbt {

struct ScheduleResult {
    bool success = false;
    Schedule schedule;
    SearchMetrics metrics;
    QualityBreakdown quality;
    std::vector<std::string> diagnostics;
};

class Scheduler {
public:
    explicit Scheduler(ProblemData data);
    ScheduleResult solve();
    const ConflictGraph& conflictGraph() const;

private:
    struct DomainValue {
        std::string instructorId;
        std::string roomId;
        std::string timeSlotId;
    };

    ProblemData data_;
    ConflictGraph graph_;
    std::vector<CourseEvent> events_;
    std::map<std::string, std::vector<DomainValue>> domains_;
    Schedule working_;
    SearchMetrics metrics_;
    std::vector<std::string> diagnostics_;

    void createEvents();
    void createDomains();
    std::string requiredRoomType(const CourseEvent& event) const;
    Assignment makeAssignment(const CourseEvent& event, const DomainValue& value) const;
    bool backtrack();
    std::optional<CourseEvent> selectUnassignedEvent();
    std::vector<DomainValue> legalValues(const CourseEvent& event);
    int remainingValueCountAfter(const CourseEvent& event, const DomainValue& value);
    int localPreferencePenalty(const CourseEvent& event, const DomainValue& value) const;
    bool forwardCheck();
    bool sameCourseOrConflict(const CourseEvent& left, const CourseEvent& right) const;
};

} // namespace cbt
