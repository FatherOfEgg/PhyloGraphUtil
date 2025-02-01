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
#include <vector>

static std::unordered_map<std::string, uint64_t> bitmaskId;

static std::unordered_set<uint64_t> computeClustersHelper(
    const Graph &g,
    const std::unordered_map<uint64_t, uint8_t> &curEdges
) {
    std::unordered_set<uint64_t> res;

    std::function<uint64_t(uint64_t)> postOrder = [&](uint64_t node) -> uint64_t {
        uint64_t bitmask = 0;

        auto leafIt = g.leafName.find(node);

        if (leafIt != g.leafName.end()) {
            uint64_t id = bitmaskId.at(leafIt->second);
            bitmask = 1 << id;
        } else {
            for (const uint64_t &c : g.adjList[node]) {
                auto it = g.reticulations.find(c);

                // Check if the current node has an edge
                // to a reticulation at child c
                if (it != g.reticulations.end()) {
                    // Child c is a reticulation, so get its parents
                    const std::vector<uint64_t> &parents = it->second;
                    uint8_t curEdge = curEdges.at(c);

                    // curEdge indexes into parents
                    // Continue if there is "no" edge from node to reticulation
                    //
                    if (parents[curEdge] != node) {
                        continue;
                    }
                }

                bitmask |= postOrder(c);
            }
        }

        res.insert(bitmask);
        return bitmask;
    };

    postOrder(g.root);

    return res;
}

static std::vector<std::unordered_set<uint64_t>> computeClusters(const Graph &g) {
    std::vector<std::unordered_set<uint64_t>> clusters;

    std::unordered_map<uint64_t, uint8_t> curEdges;
    curEdges.reserve(g.reticulations.size());

    for (const auto &p : g.reticulations) {
        curEdges[p.first] = 0;
    }

    while (true) {
        clusters.push_back(computeClustersHelper(g, curEdges));

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

    return clusters;
}

static uint64_t rfDist(
    std::unordered_set<uint64_t> clusters1,
    std::unordered_set<uint64_t> clusters2
) {
    uint64_t commonClusters = 0;

    for (const uint64_t &c1 : clusters1) {
        if (clusters2.find(c1) != clusters2.end()) {
            commonClusters++;
        }
    }

    // std::cout << "similarity: " << commonClusters << std::endl;
    // std::cout << "diff: " << clusters1.size() + clusters2.size() - 2 * commonClusters << std::endl;

    return clusters1.size() + clusters2.size() - 2 * commonClusters;
}

void robinsonFoulds(const Graph &g1, const Graph &g2) {
    if (g1.leaves.size() != g2.leaves.size()) {
        std::cerr << "Trees do not have the same number of leaves." << std::endl;
        std::exit(EXIT_FAILURE);
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
        std::exit(EXIT_FAILURE);
    }

    // Setup bitmask ids
    leaves1.insert(leaves2.begin(), leaves2.end());

    for (const std::string &l : leaves1) {
        bitmaskId[l] = bitmaskId.size();
    }

    /* if (g1.reticulations.size() != g2.reticulations.size()) {
        std::cerr << "Trees do not have the same number of reticulations." << std::endl;
        std::exit(EXIT_FAILURE);
    } */

    std::vector<std::unordered_set<uint64_t>> c1 = computeClusters(g1);
    std::vector<std::unordered_set<uint64_t>> c2 = computeClusters(g2);

    // std::cout << c1.size() << std::endl;
    // std::cout << c2.size() << std::endl;

    size_t n = c1.size();
    size_t m = c2.size();
    size_t size = std::max(n, m);

    std::vector<std::vector<double>> costMatrix(size, std::vector<double>(size, 0.0));

    uint64_t minDist = std::numeric_limits<uint64_t>::max();
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < m; j++) {
            uint64_t dist = rfDist(c1[i], c2[j]);
            
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
        if (rowsol[i] < static_cast<int>(m)) {
            uint64_t dist = static_cast<uint64_t>(costMatrix[i][rowsol[i]]);
            minDist = std::min(minDist, dist);
        }
    }

    size_t leaves = g1.leaves.size();
    double maxRFDist = 2 * (leaves - 2);
    double percentage = (static_cast<double>(minDist) / maxRFDist) * 100;

    std::cout << percentage << "% different (smallest RF dist: " << minDist << ")" << std::endl;
}
