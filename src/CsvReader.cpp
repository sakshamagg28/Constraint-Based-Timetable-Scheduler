#include "CsvReader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace cbt {
namespace {

int toInt(const std::string& value, const std::string& field) {
    try {
        return std::stoi(value);
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid integer in field '" + field + "': " + value);
    }
}

std::unordered_map<std::string, std::size_t> headerIndex(const std::vector<std::string>& header) {
    std::unordered_map<std::string, std::size_t> index;
    for (std::size_t i = 0; i < header.size(); ++i) {
        index[trim(header[i])] = i;
    }
    return index;
}

std::string get(const std::vector<std::string>& row,
                const std::unordered_map<std::string, std::size_t>& index,
                const std::string& field) {
    auto it = index.find(field);
    if (it == index.end() || it->second >= row.size()) {
        throw std::runtime_error("Missing CSV field: " + field);
    }
    return trim(row[it->second]);
}

} // namespace

std::string trim(const std::string& value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return "";
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

std::vector<std::string> splitList(const std::string& value, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(value);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        item = trim(item);
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    return result;
}

std::vector<std::vector<std::string>> CsvReader::readCsv(const std::filesystem::path& file) const {
    std::ifstream in(file);
    if (!in) {
        throw std::runtime_error("Unable to open CSV file: " + file.string());
    }

    std::vector<std::vector<std::string>> rows;
    std::string line;
    while (std::getline(in, line)) {
        if (trim(line).empty()) {
            continue;
        }
        std::vector<std::string> row;
        std::string cell;
        bool inQuotes = false;
        for (std::size_t i = 0; i < line.size(); ++i) {
            char ch = line[i];
            if (ch == '"') {
                if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                    cell.push_back('"');
                    ++i;
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (ch == ',' && !inQuotes) {
                row.push_back(trim(cell));
                cell.clear();
            } else {
                cell.push_back(ch);
            }
        }
        row.push_back(trim(cell));
        rows.push_back(row);
    }
    if (rows.empty()) {
        throw std::runtime_error("CSV file is empty: " + file.string());
    }
    return rows;
}

ProblemData CsvReader::loadProblem(const std::filesystem::path& inputDirectory) const {
    ProblemData data;

    {
        auto rows = readCsv(inputDirectory / "courses.csv");
        auto index = headerIndex(rows.front());
        for (std::size_t i = 1; i < rows.size(); ++i) {
            Course course;
            course.id = get(rows[i], index, "id");
            course.name = get(rows[i], index, "name");
            course.roomType = get(rows[i], index, "room_type");
            course.studentGroupId = get(rows[i], index, "student_group_id");
            course.enrolledStudents = toInt(get(rows[i], index, "enrolled_students"), "enrolled_students");
            course.weeklyLectures = toInt(get(rows[i], index, "weekly_lectures"), "weekly_lectures");
            course.weeklyLabs = toInt(get(rows[i], index, "weekly_labs"), "weekly_labs");
            course.eligibleInstructors = splitList(get(rows[i], index, "eligible_instructors"));
            data.courses.push_back(course);
        }
    }

    {
        auto rows = readCsv(inputDirectory / "instructors.csv");
        auto index = headerIndex(rows.front());
        for (std::size_t i = 1; i < rows.size(); ++i) {
            Instructor instructor;
            instructor.id = get(rows[i], index, "id");
            instructor.name = get(rows[i], index, "name");
            for (const auto& slot : splitList(get(rows[i], index, "available_timeslots"))) {
                instructor.availableTimeSlots.insert(slot);
            }
            for (const auto& slot : splitList(get(rows[i], index, "preferred_timeslots"))) {
                instructor.preferredTimeSlots.insert(slot);
            }
            data.instructors.push_back(instructor);
        }
    }

    {
        auto rows = readCsv(inputDirectory / "rooms.csv");
        auto index = headerIndex(rows.front());
        for (std::size_t i = 1; i < rows.size(); ++i) {
            Room room;
            room.id = get(rows[i], index, "id");
            room.name = get(rows[i], index, "name");
            room.type = get(rows[i], index, "type");
            room.capacity = toInt(get(rows[i], index, "capacity"), "capacity");
            data.rooms.push_back(room);
        }
    }

    {
        auto rows = readCsv(inputDirectory / "student_groups.csv");
        auto index = headerIndex(rows.front());
        for (std::size_t i = 1; i < rows.size(); ++i) {
            StudentGroup group;
            group.id = get(rows[i], index, "id");
            group.name = get(rows[i], index, "name");
            group.size = toInt(get(rows[i], index, "size"), "size");
            data.studentGroups.push_back(group);
        }
    }

    {
        auto rows = readCsv(inputDirectory / "timeslots.csv");
        auto index = headerIndex(rows.front());
        for (std::size_t i = 1; i < rows.size(); ++i) {
            TimeSlot slot;
            slot.id = get(rows[i], index, "id");
            slot.day = get(rows[i], index, "day");
            slot.start = get(rows[i], index, "start");
            slot.end = get(rows[i], index, "end");
            slot.dayIndex = toInt(get(rows[i], index, "day_index"), "day_index");
            slot.slotIndex = toInt(get(rows[i], index, "slot_index"), "slot_index");
            slot.late = get(rows[i], index, "late") == "1";
            data.timeSlots.push_back(slot);
        }
    }

    {
        auto rows = readCsv(inputDirectory / "constraints.csv");
        auto index = headerIndex(rows.front());
        for (std::size_t i = 1; i < rows.size(); ++i) {
            ConstraintRecord record;
            record.type = get(rows[i], index, "type");
            record.primary = get(rows[i], index, "primary");
            record.secondary = get(rows[i], index, "secondary");
            record.weight = toInt(get(rows[i], index, "weight"), "weight");
            data.constraints.push_back(record);
        }
    }

    data.buildIndexes();
    return data;
}

} // namespace cbt
