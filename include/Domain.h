#pragma once

#include <chrono>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cbt {

enum class EventType { Lecture, Lab };

std::string toString(EventType type);
EventType eventTypeFromString(const std::string& value);

struct Course {
    std::string id;
    std::string name;
    std::string roomType;
    std::string studentGroupId;
    int enrolledStudents = 0;
    int weeklyLectures = 0;
    int weeklyLabs = 0;
    std::vector<std::string> eligibleInstructors;
};

struct Instructor {
    std::string id;
    std::string name;
    std::unordered_set<std::string> availableTimeSlots;
    std::unordered_set<std::string> preferredTimeSlots;
};

struct Room {
    std::string id;
    std::string name;
    std::string type;
    int capacity = 0;
};

struct StudentGroup {
    std::string id;
    std::string name;
    int size = 0;
};

struct TimeSlot {
    std::string id;
    std::string day;
    std::string start;
    std::string end;
    int dayIndex = 0;
    int slotIndex = 0;
    bool late = false;
};

struct CourseEvent {
    std::string id;
    std::string courseId;
    EventType type = EventType::Lecture;
    int occurrence = 0;
};

struct Assignment {
    std::string eventId;
    std::string courseId;
    std::string courseName;
    EventType eventType = EventType::Lecture;
    std::string instructorId;
    std::string instructorName;
    std::string roomId;
    std::string roomName;
    std::string timeSlotId;
    std::string day;
    std::string start;
    std::string end;
    std::string studentGroupId;
    std::string studentGroupName;
    int enrolledStudents = 0;
};

struct Constraint {
    std::string type;
    std::string primary;
    std::string secondary;
    int weight = 1;
};

using ConstraintRecord = Constraint;

struct ProblemData {
    std::vector<Course> courses;
    std::vector<Instructor> instructors;
    std::vector<Room> rooms;
    std::vector<StudentGroup> studentGroups;
    std::vector<TimeSlot> timeSlots;
    std::vector<ConstraintRecord> constraints;

    std::unordered_map<std::string, Course> courseById;
    std::unordered_map<std::string, Instructor> instructorById;
    std::unordered_map<std::string, Room> roomById;
    std::unordered_map<std::string, StudentGroup> groupById;
    std::unordered_map<std::string, TimeSlot> timeSlotById;
    std::unordered_map<std::string, std::unordered_set<std::string>> blockedCourseTimeSlots;
    std::set<std::pair<std::string, std::string>> explicitCourseConflicts;

    void buildIndexes();
    const Course& course(const std::string& id) const;
    const Instructor& instructor(const std::string& id) const;
    const Room& room(const std::string& id) const;
    const StudentGroup& group(const std::string& id) const;
    const TimeSlot& timeSlot(const std::string& id) const;
};

struct SearchMetrics {
    long long assignmentsAttempted = 0;
    long long constraintChecks = 0;
    long long backtracks = 0;
    long long searchNodesVisited = 0;
    long long deadEnds = 0;
    double runtimeSeconds = 0.0;
    double finalScore = 0.0;
};

} // namespace cbt
