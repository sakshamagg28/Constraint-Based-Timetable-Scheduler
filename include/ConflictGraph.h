#pragma once

#include "Domain.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace cbt {

class ConflictGraph {
public:
    void build(const ProblemData& data);
    bool hasEdge(const std::string& leftCourseId, const std::string& rightCourseId) const;
    int degree(const std::string& courseId) const;
    const std::set<std::string>& neighbors(const std::string& courseId) const;
    std::vector<std::string> nodes() const;
    std::size_t edgeCount() const;

private:
    void addEdge(const std::string& left, const std::string& right);
    std::map<std::string, std::set<std::string>> adjacency_;
};

} // namespace cbt
