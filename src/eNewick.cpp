#include "eNewick.h"
#include "graph.h"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <set>
#include <string>
#include <vector>

ENewickGraph::ENewickGraph()
:mIncludeInternalNames(true) {}

ENewickGraph::ENewickGraph(const Graph &other)
: Graph(other), mIncludeInternalNames(true) {}

ENewickGraph &ENewickGraph::operator=(const Graph &other) {
    if (this != &other) {
        Graph::operator=(other);
    }

    return *this;
}

void ENewickGraph::open(const std::string &file) {

}

void ENewickGraph::save(const std::string &filename) const {
    std::cout << "\nSaving as ENewick" << std::endl;


    std::vector<std::string> roots = getRoot();

    if (roots.empty()) {
        std::cerr << "Failed to save to '" << filename << "'. Graph has no root."<< std::endl;
        std::exit(1);
    }

    std::unordered_map<std::string, std::string> hybrids = getHybrids();

    std::string res = dfs(roots[0], hybrids);
    std::cout << res << ";" << std::endl;
}

void ENewickGraph::save(const std::string &filename, bool includeInternalNames) {
    mIncludeInternalNames = includeInternalNames;
    save(filename);
}

void ENewickGraph::print() const {

}

std::string ENewickGraph::dfs(const std::string &nodeID, const std::unordered_map<std::string, std::string> &hybrids) const {
    auto it = mEdges.find(nodeID);
    if (it == mEdges.end()
    ||  it->second.empty()) {
        auto n = std::find_if(
            mNodes.begin(),
            mNodes.end(),
            [&nodeID](const Node &node) {
                return node.id == nodeID;
            }
        );
        return n->label;
    }

    std::vector<Edge> neighbors = it->second;
    std::vector<std::string> out;

    for (const auto &e : neighbors) {
        out.push_back(dfs(e.target, hybrids));
    }

    std::string res = "(" + out[0];

    for (size_t i = 1; i < out.size(); i++) {
        res += "," + out[1];
    }
    res += ")";

    if (mIncludeInternalNames) {
        res += nodeID;
    }

    auto hybrid = hybrids.find(nodeID);
    if (hybrid != hybrids.end()) {
        res += hybrid->second;
    }

    return res;
}

std::vector<std::string> ENewickGraph::getRoot() const {
    std::vector<std::string> roots;
    std::set<std::string> incomingEdges;

    for (const auto &pair : mEdges) {
        for (const auto &e : pair.second) {
            incomingEdges.insert(e.target);
        }
    }

    for (const auto &n : mNodes) {
        if (incomingEdges.find(n.id) == incomingEdges.end()) {
            roots.push_back(n.id);
        }
    }

    return roots;
}

std::unordered_map<std::string, std::string> ENewickGraph::getHybrids() const {
    std::unordered_map<std::string, std::string> hybrids;
    std::unordered_map<std::string, unsigned int> incomingCount;

    for (const auto &e : mEdges) {
        for (const auto &x : e.second) {
            incomingCount[x.target]++;
        }
    }

    unsigned int hybridID = 1;
    for (const auto &n : incomingCount) {
        if (n.second >= 2) {
            char hybrid[100];
            std::snprintf(hybrid, 100, "#H%d", hybridID);
            hybrids[n.first] = hybrid;
        }
    }

    return hybrids;
}
