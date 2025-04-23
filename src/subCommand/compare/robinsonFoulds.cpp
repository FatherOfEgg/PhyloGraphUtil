#include "robinsonFoulds.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "util/lap.h"

using PSW = std::vector<std::pair<uint64_t, uint64_t>>;

struct ClusterTable {
public:
    ClusterTable(const Graph &g, const PSW &psw) {
        uint64_t leafCode = 0;
        internalLabels.reserve(g.leafName.size());

        uint64_t rightLeaf = 0;

        for (size_t i = 0; i < psw.size(); i++) {
            const std::pair<uint64_t, uint64_t> p = psw[i];

            // If leaf
            if (p.second == 0) {
                rightLeaf = leafCode;
                internalLabels[p.first] = leafCode;
                leafCode++;
            } else {
                uint64_t leftLeafIndex = psw[i - p.second].first;
                uint64_t leftLeaf = internalLabels[leftLeafIndex];

                ct[leftLeaf].insert(rightLeaf);
            }
        }

    }

    uint64_t encode(uint64_t v) {
        return internalLabels.at(v);
    }

    bool isClust(uint64_t L, uint64_t R) {
        auto itL = ct.find(L);

        if (itL == ct.end()) {
            return false;
        }

        return itL->second.find(R) != itL->second.end();
    }

public:
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> ct;
    std::unordered_map<uint64_t, uint64_t> internalLabels;
};

// Post order sequence with weights (PSW)
static PSW genPSWHelper(
    const Graph &g,
    const std::unordered_map<uint64_t, uint8_t> &curEdges
) {
    PSW res;

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

// COMCLUST
static uint64_t rf_dist(
    const Graph &g1, const PSW &psw1,
    const Graph &g2, const PSW &psw2
) {
    uint64_t res = 0;

    ClusterTable ct(g1, psw1);

    std::stack<std::pair<uint64_t, uint64_t>> s;

    for (size_t i = 0; i < psw2.size(); i++) {
        const std::pair<uint64_t, uint64_t> p = psw2[i];

        uint64_t w = p.second;

        // If leaf
        if (w == 0) {
            s.emplace(ct.encode(p.first), ct.encode(p.first));
        } else {
            std::pair<uint64_t, uint64_t> lr = std::make_pair(UINT64_MAX, 0);
            uint64_t n = 0;

            do {
                std::pair<uint64_t, uint64_t> temp = s.top();
                s.pop();

                lr.first = std::min(lr.first, temp.first);
                lr.second = std::max(lr.second, temp.second);

                n++;
                w--;
            } while(w != 0);

            s.push(lr);

            if (n == lr.second - lr.first + 1) {
                std::cout << "(" << lr.first << "," << lr.second;
                // std::cout << "," << n;
                std::cout << ")" << std::endl;
            }
        }
    }

    return res;
}

/* static uint64_t rfDist(
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
} */

void robinsonFoulds(const Graph &g1, const Graph &g2) {
    // auto x = genPSWs(g1);
    PSW psw = {
        std::make_pair(10, 0),
        std::make_pair(7, 0),
        std::make_pair(15, 2),
        std::make_pair(8, 0),
        std::make_pair(11, 0),
        std::make_pair(16, 2),
        std::make_pair(6, 0),
        std::make_pair(19, 4),
        std::make_pair(12, 0),
        std::make_pair(4, 0),
        std::make_pair(2, 0),
        std::make_pair(1, 0),
        std::make_pair(17, 2),
        std::make_pair(20, 4),
        std::make_pair(21, 6),
        std::make_pair(14, 0),
        std::make_pair(5, 0),
        std::make_pair(9, 0),
        std::make_pair(13, 0),
        std::make_pair(18, 3),
        std::make_pair(3, 0),
        std::make_pair(22, 21),
    };

    // for (const auto &e : g1.leafName) {
    //     std::cout << e.first << ": " << e.second << std::endl;
    // }
    // std::cout << std::endl;

    // ClusterTable a(g1, psw);
    //
    // for (const auto &x : a.ct) {
    //     for (const auto &y : x.second) {
    //         std::cout << "(" << x.first << "," << y << ")" << std::endl;
    //     }
    // }

    PSW psw1 {
        std::make_pair(0, 0),
        std::make_pair(1, 0),
        std::make_pair(2, 2),
    };

    PSW psw2 {
        std::make_pair(0, 0),
        std::make_pair(1, 0),
        std::make_pair(2, 2),
    };

    rf_dist(g1, psw1, g2, psw2);

    std::exit(0);

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
    /* std::unordered_map<std::string, uint64_t> bitmaskId;

    leaves1.insert(leaves2.begin(), leaves2.end());

    for (const std::string &l : leaves1) {
        bitmaskId[l] = bitmaskId.size();
    } */

    /* if (g1.reticulations.size() != g2.reticulations.size()) {
        std::cerr << "Trees do not have the same number of reticulations." << std::endl;
        std::exit(EXIT_FAILURE);
    } */

    std::vector<PSW> p1 = genPSWs(g1);
    std::vector<PSW> p2 = genPSWs(g2);

    size_t n = p1.size();
    size_t m = p2.size();
    size_t size = std::max(n, m);

    std::vector<std::vector<double>> costMatrix(size, std::vector<double>(size, 0.0));

    uint64_t minDist = std::numeric_limits<uint64_t>::max();
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < m; j++) {
            uint64_t dist = rf_dist(g1, p1[i], g2, p2[j]);
            
            // If we find a perfect match
            /* if (dist == 0) {
                std::cout << "0% different (smallest RF dist: 0)" << std::endl;
                return;
            } */

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
