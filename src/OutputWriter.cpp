#include "OutputWriter.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace cbt {
namespace {

std::string csvEscape(const std::string& value) {
    if (value.find_first_of(",\"\n") == std::string::npos) {
        return value;
    }
    std::string escaped = "\"";
    for (char ch : value) {
        if (ch == '"') {
            escaped += "\"\"";
        } else {
            escaped += ch;
        }
    }
    escaped += '"';
    return escaped;
}

std::string slotLabel(const Assignment& assignment) {
    return assignment.day + " " + assignment.start + "-" + assignment.end;
}

void sortAssignments(std::vector<Assignment>& assignments) {
    std::sort(assignments.begin(), assignments.end(), [](const Assignment& a, const Assignment& b) {
        if (a.day != b.day) {
            static const std::vector<std::string> days = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
            auto ai = std::find(days.begin(), days.end(), a.day);
            auto bi = std::find(days.begin(), days.end(), b.day);
            return ai < bi;
        }
        return a.start < b.start;
    });
}

void printTable(std::ostream& os, const std::vector<std::string>& headers, const std::vector<std::vector<std::string>>& rows) {
    std::vector<std::size_t> widths(headers.size(), 0);
    for (std::size_t i = 0; i < headers.size(); ++i) {
        widths[i] = headers[i].size();
    }
    for (const auto& row : rows) {
        for (std::size_t i = 0; i < row.size(); ++i) {
            widths[i] = std::max(widths[i], row[i].size());
        }
    }

    auto border = [&]() {
        os << '+';
        for (auto width : widths) {
            os << std::string(width + 2, '-') << '+';
        }
        os << '\n';
    };

    border();
    os << '|';
    for (std::size_t i = 0; i < headers.size(); ++i) {
        os << ' ' << std::left << std::setw(static_cast<int>(widths[i])) << headers[i] << " |";
    }
    os << '\n';
    border();
    for (const auto& row : rows) {
        os << '|';
        for (std::size_t i = 0; i < headers.size(); ++i) {
            const auto value = i < row.size() ? row[i] : "";
            os << ' ' << std::left << std::setw(static_cast<int>(widths[i])) << value << " |";
        }
        os << '\n';
    }
    border();
}

void writeFullCsv(const std::filesystem::path& file, const std::vector<Assignment>& assignments) {
    std::ofstream out(file);
    out << "event_id,course,type,instructor,student_group,room,day,start,end,timeslot\n";
    for (const auto& assignment : assignments) {
        out << csvEscape(assignment.eventId) << ','
            << csvEscape(assignment.courseName) << ','
            << csvEscape(toString(assignment.eventType)) << ','
            << csvEscape(assignment.instructorName) << ','
            << csvEscape(assignment.studentGroupName) << ','
            << csvEscape(assignment.roomName) << ','
            << csvEscape(assignment.day) << ','
            << csvEscape(assignment.start) << ','
            << csvEscape(assignment.end) << ','
            << csvEscape(assignment.timeSlotId) << '\n';
    }
}

} // namespace

OutputWriter::OutputWriter(const ProblemData& data) : data_(data) {}

void OutputWriter::printAll(std::ostream& os, const Schedule& schedule, const SearchMetrics& metrics) const {
    printInstructorSchedules(os, schedule);
    printStudentSchedules(os, schedule);
    printRoomSchedules(os, schedule);

    os << "\nSearch Metrics\n";
    printTable(os,
               {"Metric", "Value"},
               {{"Assignments attempted", std::to_string(metrics.assignmentsAttempted)},
                {"Constraint checks", std::to_string(metrics.constraintChecks)},
                {"Backtracks", std::to_string(metrics.backtracks)},
                {"Search nodes visited", std::to_string(metrics.searchNodesVisited)},
                {"Dead ends", std::to_string(metrics.deadEnds)},
                {"Runtime seconds", std::to_string(metrics.runtimeSeconds)},
                {"Final quality score", std::to_string(metrics.finalScore)}});
}

void OutputWriter::printInstructorSchedules(std::ostream& os, const Schedule& schedule) const {
    os << "\nInstructor Timetables\n";
    for (const auto& instructor : data_.instructors) {
        auto assignments = schedule.byInstructor(instructor.id);
        sortAssignments(assignments);
        std::vector<std::vector<std::string>> rows;
        for (const auto& item : assignments) {
            rows.push_back({item.courseName + " (" + toString(item.eventType) + ")",
                            item.studentGroupName,
                            item.roomName,
                            slotLabel(item)});
        }
        os << "\n" << instructor.name << "\n";
        printTable(os, {"Course", "Group", "Room", "Time Slot"}, rows);
    }
}

void OutputWriter::printStudentSchedules(std::ostream& os, const Schedule& schedule) const {
    os << "\nStudent Group Timetables\n";
    for (const auto& group : data_.studentGroups) {
        auto assignments = schedule.byStudentGroup(group.id);
        sortAssignments(assignments);
        std::vector<std::vector<std::string>> rows;
        for (const auto& item : assignments) {
            rows.push_back({item.courseName + " (" + toString(item.eventType) + ")",
                            item.instructorName,
                            item.roomName,
                            slotLabel(item)});
        }
        os << "\n" << group.name << "\n";
        printTable(os, {"Course", "Teacher", "Room", "Time Slot"}, rows);
    }
}

void OutputWriter::printRoomSchedules(std::ostream& os, const Schedule& schedule) const {
    os << "\nRoom Timetables\n";
    for (const auto& room : data_.rooms) {
        auto assignments = schedule.byRoom(room.id);
        sortAssignments(assignments);
        std::vector<std::vector<std::string>> rows;
        for (const auto& item : assignments) {
            rows.push_back({item.courseName + " (" + toString(item.eventType) + ")",
                            item.instructorName,
                            item.studentGroupName,
                            slotLabel(item)});
        }
        os << "\n" << room.name << "\n";
        printTable(os, {"Course", "Teacher", "Group", "Time Slot"}, rows);
    }
}

void OutputWriter::exportCsv(const std::filesystem::path& outputDirectory, const Schedule& schedule) const {
    std::filesystem::create_directories(outputDirectory);
    auto assignments = schedule.assignments();
    sortAssignments(assignments);
    writeFullCsv(outputDirectory / "full_schedule.csv", assignments);

    {
        std::ofstream out(outputDirectory / "instructor_schedule.csv");
        out << "instructor,course,type,student_group,room,day,start,end\n";
        for (const auto& item : assignments) {
            out << csvEscape(item.instructorName) << ',' << csvEscape(item.courseName) << ','
                << csvEscape(toString(item.eventType)) << ',' << csvEscape(item.studentGroupName) << ','
                << csvEscape(item.roomName) << ',' << csvEscape(item.day) << ','
                << csvEscape(item.start) << ',' << csvEscape(item.end) << '\n';
        }
    }

    {
        std::ofstream out(outputDirectory / "student_schedule.csv");
        out << "student_group,course,type,instructor,room,day,start,end\n";
        for (const auto& item : assignments) {
            out << csvEscape(item.studentGroupName) << ',' << csvEscape(item.courseName) << ','
                << csvEscape(toString(item.eventType)) << ',' << csvEscape(item.instructorName) << ','
                << csvEscape(item.roomName) << ',' << csvEscape(item.day) << ','
                << csvEscape(item.start) << ',' << csvEscape(item.end) << '\n';
        }
    }

    {
        std::ofstream out(outputDirectory / "room_schedule.csv");
        out << "room,course,type,instructor,student_group,day,start,end\n";
        for (const auto& item : assignments) {
            out << csvEscape(item.roomName) << ',' << csvEscape(item.courseName) << ','
                << csvEscape(toString(item.eventType)) << ',' << csvEscape(item.instructorName) << ','
                << csvEscape(item.studentGroupName) << ',' << csvEscape(item.day) << ','
                << csvEscape(item.start) << ',' << csvEscape(item.end) << '\n';
        }
    }
}

} // namespace cbt
