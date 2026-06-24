#pragma once

#include "Domain.h"

#include <filesystem>
#include <string>
#include <vector>

namespace cbt {

class CsvReader {
public:
    ProblemData loadProblem(const std::filesystem::path& inputDirectory) const;

private:
    std::vector<std::vector<std::string>> readCsv(const std::filesystem::path& file) const;
};

std::vector<std::string> splitList(const std::string& value, char delimiter = '|');
std::string trim(const std::string& value);

} // namespace cbt
