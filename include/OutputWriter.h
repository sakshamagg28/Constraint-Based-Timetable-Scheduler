#pragma once

#include "Domain.h"
#include "Schedule.h"

#include <filesystem>
#include <ostream>

namespace cbt {

class OutputWriter {
public:
    explicit OutputWriter(const ProblemData& data);

    void printAll(std::ostream& os, const Schedule& schedule, const SearchMetrics& metrics) const;
    void exportCsv(const std::filesystem::path& outputDirectory, const Schedule& schedule) const;

private:
    const ProblemData& data_;

    void printInstructorSchedules(std::ostream& os, const Schedule& schedule) const;
    void printStudentSchedules(std::ostream& os, const Schedule& schedule) const;
    void printRoomSchedules(std::ostream& os, const Schedule& schedule) const;
};

} // namespace cbt
