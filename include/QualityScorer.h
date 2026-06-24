#pragma once

#include "Domain.h"
#include "Schedule.h"

#include <string>
#include <vector>

namespace cbt {

struct QualityBreakdown {
    double rawPenalty = 0.0;
    double score = 0.0;
    std::vector<std::string> notes;
};

class QualityScorer {
public:
    explicit QualityScorer(const ProblemData& data);
    QualityBreakdown score(const Schedule& schedule) const;

private:
    const ProblemData& data_;
};

} // namespace cbt
