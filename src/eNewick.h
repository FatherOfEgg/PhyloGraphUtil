#pragma once

#include "graph.h"
#include <string>
#include <unordered_map>

class ENewickGraph : public Graph {
public:
    ENewickGraph();
    ENewickGraph(const Graph &other);

    ENewickGraph &operator=(const Graph &other);

    void open(const std::string &file) override;
    void save(const std::string &filename) const override;
    void save(const std::string &filename, bool includeInternalNames);
    void print() const override;

private:
    std::string dfs(const std::string &nodeID, const std::unordered_map<std::string, std::string> &hybrids) const;
    std::vector<std::string> getRoot() const;
    std::unordered_map<std::string, std::string> getHybrids() const;

private:
    bool mIncludeInternalNames;
};
