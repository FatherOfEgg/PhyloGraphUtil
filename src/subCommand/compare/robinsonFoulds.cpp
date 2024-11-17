#include "robinsonFoulds.h"
#include "lap.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

static uint64_t getRoot(const std::vector<std::vector<uint64_t>> &adjList) {
    std::vector<uint64_t> inDegree(adjList.size());

    for (const auto &n : adjList) {
        for (const auto &t : n) {
            inDegree[t]++;
        }
    }

    auto it = std::find(inDegree.begin(), inDegree.end(), 0);

    if (it == inDegree.end()) {
        std::cerr << "Couldn't find root." << std::endl;
        std::exit(1);
    }

    return std::distance(inDegree.begin(), it);
}

static std::vector<std::unordered_set<std::string>> getSplitsHelper(
    const Graph &g,
    const std::unordered_map<uint64_t, uint8_t> &curEdges
) {
    uint64_t root = getRoot(g.adjList);

    std::vector<std::unordered_set<std::string>> splits;
    std::vector<std::unordered_set<std::string>> subtreeLeaves(g.adjList.size());

    std::function<std::unordered_set<std::string>(uint64_t)> dfs = [&](uint64_t node) -> std::unordered_set<std::string> {
        if (g.leafName.find(node) != g.leafName.end()) {
            // Leaf is more or less a subtree of itself
            subtreeLeaves[node].insert(g.leafName.at(node));
            return subtreeLeaves[node];
        }

        for (const uint64_t &c : g.adjList[node]) {
            // Get this node's children's subtree and the leaves that they each can reach

            // Check if node has an edge to a reticulation
            auto it = g.reticulations.find(c);
            if (it != g.reticulations.end()) {
                const std::vector<uint64_t> &parents = it->second;
                uint8_t curEdge = curEdges.at(c);

                // Continue if there is "no" edge from node to reticulation
                if (parents[curEdge] != node) {
                    continue;
                }
            }

            std::unordered_set<std::string> children = dfs(c);
            subtreeLeaves[node].insert(children.begin(), children.end());
        }

        if (node != root) {
            splits.push_back(subtreeLeaves[node]);
        }

        // Return the leaves that this node can reach
        return subtreeLeaves[node];
    };

    dfs(root);
    return splits;
}

static std::vector<std::vector<std::unordered_set<std::string>>> getSplits(const Graph &g) {
    std::vector<std::vector<std::unordered_set<std::string>>> res;

    std::unordered_map<uint64_t, uint8_t> curEdges;

    for (const auto &p : g.reticulations) {
        curEdges[p.first] = 0;
    }

    while (true) {
        res.push_back(getSplitsHelper(g, curEdges));

        auto it = curEdges.begin();
        while (it != curEdges.end()) {
            if (it->second < g.reticulations.at(it->first).size() - 1) {
                it->second++;
                break;
            } else {
                it->second = 0;
            }

            it++;
        }

        if (it == curEdges.end()) {
            break;
        }
    }

    return res;
}

static uint64_t rf_dist(
    const std::vector<std::unordered_set<std::string>> &splits1,
    const std::vector<std::unordered_set<std::string>> &splits2
) {
    // TODO: Figure out a better way to do this, maybe bit manip?
    auto concatSplit = [](const std::unordered_set<std::string> &split) {
        std::string res;

        for (const auto &leaf : split) {
            res += leaf + ",";
        }

        return res;
    };

    std::unordered_set<std::string> s1;
    for (const auto &s : splits1) {
        s1.insert(concatSplit(s));
    }

    std::unordered_set<std::string> s2;
    for (const auto &s : splits2) {
        s2.insert(concatSplit(s));
    }

    uint64_t commonSplits = 0;
    for (const auto& split : s1) {
        if (s2.find(split) != s2.end()) {
            commonSplits++;
        }
    }

    uint64_t rf_distance = s1.size() + s2.size() - 2 * commonSplits;
    return rf_distance;
}

void robinsonFoulds(const Graph &g1, const Graph &g2) {
    if (g1.leaves.size() != g2.leaves.size()) {
        std::cerr << "Trees do not have the same number of leaves." << std::endl;
        std::exit(1);
    }

    std::unordered_set<std::string> leaves1;
    for (const auto &p : g1.leafName) {
        leaves1.insert(p.second);
    }

    std::unordered_set<std::string> leaves2;
    for (const auto &p : g2.leafName) {
        leaves2.insert(p.second);
    }

    if (leaves1 != leaves2) {
        std::cerr << "Trees do not have the same leaf names." << std::endl;
        std::exit(1);
    }

    if (g1.reticulations.size() != g2.reticulations.size()) {
        std::cerr << "Trees do not have the same number of reticulations." << std::endl;
        std::exit(1);
    }

    auto splits1 = getSplits(g1);
    auto splits2 = getSplits(g2);

    size_t n = splits1.size();
    size_t m = splits2.size();
    size_t size = std::max(n, m);

    std::vector<std::vector<double>> costMatrix(size, std::vector<double>(size, 0.0));

    uint64_t minDist = std::numeric_limits<uint64_t>::max();
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < m; j++) {
            uint64_t dist = rf_dist(splits1[i], splits2[j]);
            
            // If we find a perfect match
            if (dist == 0) {
                std::cout << "0% different (smallest RF dist: 0)" << std::endl;
                return;
            }

            costMatrix[i][j] = static_cast<double>(dist);

            if (dist < minDist) {
                minDist = dist;
            }
        }
    }

    // Pad with double::max if the matrix is not square
    double doubleMax = std::numeric_limits<double>::max();
    for (size_t i = n; i < size; i++) {
        std::fill(costMatrix[i].begin(), costMatrix[i].end(), doubleMax);
    }
    for (size_t i = 0; i < n; i++) {
        std::fill(costMatrix[i].begin() + m, costMatrix[i].end(), doubleMax);
    }

    std::vector<int> rowsol(size);
    std::vector<int> colsol(size);
    std::vector<double> u(size);
    std::vector<double> v(size);

    lap(size, costMatrix, rowsol, colsol, u, v);

    for (size_t i = 0; i < n; i++) {
        // Check if it's a valid assignment
        if (rowsol[i] < m) {
            uint64_t dist = static_cast<uint64_t>(costMatrix[i][rowsol[i]]);
            minDist = std::min(minDist, dist);
        }
    }

    size_t leaves = g1.leaves.size();
    double maxRFDist = 2 * (leaves - 2);
    double percentage = (static_cast<double>(minDist) / maxRFDist) * 100;

    std::cout << percentage << "% different (smallest RF dist: " << minDist << ")" << std::endl;
}
