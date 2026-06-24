#include "ConflictGraph.h"
#include "CsvReader.h"
#include "Scheduler.h"
#include "Validator.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace {

void check(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

cbt::Assignment sampleAssignment(const cbt::ProblemData& data,
                                 const std::string& eventId,
                                 const std::string& courseId,
                                 cbt::EventType type,
                                 const std::string& instructorId,
                                 const std::string& roomId,
                                 const std::string& slotId) {
    const auto& course = data.course(courseId);
    const auto& instructor = data.instructor(instructorId);
    const auto& room = data.room(roomId);
    const auto& slot = data.timeSlot(slotId);
    const auto& group = data.group(course.studentGroupId);

    cbt::Assignment assignment;
    assignment.eventId = eventId;
    assignment.courseId = course.id;
    assignment.courseName = course.name;
    assignment.eventType = type;
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

void testCsvParsing(const cbt::ProblemData& data) {
    check(data.courses.size() >= 10, "Expected at least 10 courses.");
    check(data.instructors.size() == 5, "Expected 5 instructors.");
    check(data.rooms.size() == 5, "Expected 5 rooms.");
    check(data.studentGroups.size() == 4, "Expected 4 student groups.");
    check(data.timeSlots.size() >= 6, "Expected at least 6 time slots.");
    check(data.courseById.count("CS101") == 1, "CS101 should be indexed.");
}

void testConflictGraph(const cbt::ProblemData& data) {
    cbt::ConflictGraph graph;
    graph.build(data);
    check(graph.hasEdge("CS101", "CS102"), "Courses for the same group should conflict.");
    check(graph.hasEdge("CS101", "AI401"), "Explicit course conflict should be represented.");
    check(graph.degree("CS101") > 0, "CS101 should have graph degree.");
}

void testValidationConflicts(const cbt::ProblemData& data) {
    cbt::Validator validator(data);
    cbt::Schedule schedule;
    auto first = sampleAssignment(data, "E1", "CS101", cbt::EventType::Lecture, "I1", "R2", "MON_10");
    check(validator.canPlace(schedule, first), "First assignment should be legal.");
    schedule.add(first);

    auto instructorConflict = sampleAssignment(data, "E2", "CS201", cbt::EventType::Lecture, "I1", "R3", "MON_10");
    check(!validator.canPlace(schedule, instructorConflict), "Instructor double-booking should be rejected.");

    auto roomConflict = sampleAssignment(data, "E3", "HU101", cbt::EventType::Lecture, "I2", "R2", "MON_10");
    check(!validator.canPlace(schedule, roomConflict), "Room double-booking should be rejected.");

    auto groupConflict = sampleAssignment(data, "E4", "CS102", cbt::EventType::Lecture, "I2", "R3", "MON_10");
    check(!validator.canPlace(schedule, groupConflict), "Student group double-booking should be rejected.");

    auto capacityFailure = sampleAssignment(data, "E5", "CS201", cbt::EventType::Lecture, "I3", "R5", "MON_11");
    check(!validator.canPlace(schedule, capacityFailure), "Capacity/type mismatch should be rejected.");
}

void testScheduler(const cbt::ProblemData& data) {
    cbt::Scheduler scheduler(data);
    auto result = scheduler.solve();
    check(result.success, "Scheduler should solve the sample dataset.");
    check(result.schedule.size() == 28, "Sample dataset should produce 28 scheduled events.");
    check(result.metrics.assignmentsAttempted > 0, "Metrics should track assignment attempts.");
    check(result.metrics.finalScore > 0.0, "Schedule should receive a positive quality score.");
}

} // namespace

int main() {
    try {
        cbt::CsvReader reader;
        auto data = reader.loadProblem(std::filesystem::path("data"));
        testCsvParsing(data);
        testConflictGraph(data);
        testValidationConflicts(data);
        testScheduler(data);
        std::cout << "All scheduler tests passed.\n";
    } catch (const std::exception& ex) {
        std::cerr << "Test failure: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
