#pragma once

#include "Domain.h"
#include "Schedule.h"

#include <string>
#include <vector>

namespace cbt {

struct ValidationResult {
    bool valid = true;
    std::vector<std::string> errors;

    void addError(const std::string& message);
};

class Validator {
public:
    explicit Validator(const ProblemData& data);

    bool canPlace(const Schedule& schedule, const Assignment& candidate, SearchMetrics* metrics = nullptr) const;
    ValidationResult validateComplete(const Schedule& schedule, const std::vector<CourseEvent>& events) const;

private:
    const ProblemData& data_;
};

} // namespace cbt
