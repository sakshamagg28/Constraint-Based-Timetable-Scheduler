#include "ConflictGraph.h"

#include <algorithm>

namespace cbt {

void ConflictGraph::build(const ProblemData& data) {
    adjacency_.clear();
    for (const auto& course : data.courses) {
        adjacency_[course.id];
    }

    for (std::size_t i = 0; i < data.courses.size(); ++i) {
        for (std::size_t j = i + 1; j < data.courses.size(); ++j) {
            const auto& left = data.courses[i];
            const auto& right = data.courses[j];

            bool conflict = left.studentGroupId == right.studentGroupId;
            for (const auto& instructor : left.eligibleInstructors) {
                if (std::find(right.eligibleInstructors.begin(), right.eligibleInstructors.end(), instructor)
                    != right.eligibleInstructors.end()) {
                    conflict = true;
                    break;
                }
            }

            auto a = std::min(left.id, right.id);
            auto b = std::max(left.id, right.id);
            if (data.explicitCourseConflicts.count({a, b}) > 0) {
                conflict = true;
            }

            if (conflict) {
                addEdge(left.id, right.id);
            }
        }
    }
}

void ConflictGraph::addEdge(const std::string& left, const std::string& right) {
    if (left == right) {
        return;
    }
    adjacency_[left].insert(right);
    adjacency_[right].insert(left);
}

bool ConflictGraph::hasEdge(const std::string& leftCourseId, const std::string& rightCourseId) const {
    auto it = adjacency_.find(leftCourseId);
    return it != adjacency_.end() && it->second.count(rightCourseId) > 0;
}

int ConflictGraph::degree(const std::string& courseId) const {
    auto it = adjacency_.find(courseId);
    return it == adjacency_.end() ? 0 : static_cast<int>(it->second.size());
}

const std::set<std::string>& ConflictGraph::neighbors(const std::string& courseId) const {
    static const std::set<std::string> empty;
    auto it = adjacency_.find(courseId);
    return it == adjacency_.end() ? empty : it->second;
}

std::vector<std::string> ConflictGraph::nodes() const {
    std::vector<std::string> result;
    for (const auto& [node, _] : adjacency_) {
        result.push_back(node);
    }
    return result;
}

std::size_t ConflictGraph::edgeCount() const {
    std::size_t total = 0;
    for (const auto& [_, neighbors] : adjacency_) {
        total += neighbors.size();
    }
    return total / 2;
}

} // namespace cbt
