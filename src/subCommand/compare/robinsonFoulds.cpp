#include "robinsonFoulds.h"

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

#include "util/cluster.h"
#include "util/lap.h"

using PSW = std::vector<std::pair<uint64_t, uint64_t>>;

// Post order sequence with weights (PSW)
static PSW genPSWHelper(
    const Graph &g,
    const std::unordered_map<uint64_t, uint8_t> &curEdges
) {
    std::vector<std::pair<uint64_t, uint64_t>> res;

    std::function<uint64_t(uint64_t)> postOrder = [&](uint64_t node) -> uint64_t {
        uint64_t weight = 0;

        auto leafIt = g.leafName.find(node);

        if (leafIt == g.leafName.end()) {
            for (const uint64_t &c : g.adjList[node]) {
                auto it = g.reticulations.find(c);

                // Check if the current node has an edge
                // to a reticulation at child c
                if (it != g.reticulations.end()) {
                    // Child c is a reticulation, so get its parents
                    const std::vector<uint64_t> &parents = it->second;
                    uint8_t curEdge = curEdges.at(c);

                    // Continue if there's "no" edge from reticulation's parent
                    // to the reticulation c
                    if (parents[curEdge] != node) {
                        continue;
                    }
                }

                weight += postOrder(c);
            }
        }

        res.push_back(std::make_pair(node, weight));
        return weight + 1;
    };

    postOrder(g.root);

    return res;
}

static std::vector<PSW> genPSWs(
    const Graph &g
) {
    std::vector<PSW> psw;

    std::unordered_map<uint64_t, uint8_t> curEdges;
    curEdges.reserve(g.reticulations.size());

    for (const auto &p : g.reticulations) {
        curEdges[p.first] = 0;
    }

    while (true) {
        psw.emplace_back(genPSWHelper(g, curEdges));

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

    return psw;
}

static uint64_t rfDist(
    BitmaskSet clusters1,
    BitmaskSet clusters2
) {
    uint64_t commonClusters = 0;

    for (const Bitmask &c1 : clusters1) {
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
    std::unordered_map<std::string, uint64_t> bitmaskId;

    leaves1.insert(leaves2.begin(), leaves2.end());

    for (const std::string &l : leaves1) {
        bitmaskId[l] = bitmaskId.size();
    }

    /* if (g1.reticulations.size() != g2.reticulations.size()) {
        std::cerr << "Trees do not have the same number of reticulations." << std::endl;
        std::exit(EXIT_FAILURE);
    } */

    std::vector<BitmaskSet> c1 = computeClusters(g1, bitmaskId);
    std::vector<BitmaskSet> c2 = computeClusters(g2, bitmaskId);

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

    std::cout << percentage << "% difference between the two graphs" << std::endl;
    std::cout << "Sum of smallest RF distances between trees: " << minDist << std::endl;
    std::cout << "Sum of total number of non-trivial splits: " << maxRFDist << std::endl;
}
