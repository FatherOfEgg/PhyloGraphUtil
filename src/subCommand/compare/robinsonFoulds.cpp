#include "robinsonFoulds.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "util/clusterTable.h"
#include "util/lap.h"
#include "util/psw.h"

// COMCLUST
static uint64_t rfDist(
    const ClusterTable &ct,
    const Graph &g2, const PSW &psw2
) {
    uint64_t res;

    std::stack<LRNW> s;

    for (size_t i = 0; i < psw2.size(); i++) {
        const std::pair<uint64_t, uint64_t> p = psw2[i];

        uint64_t w = p.second;

        // If leaf
        if (w == 0) {
            std::string leafName = g2.leafName.at(p.first);
            uint64_t encode = ct.encode(leafName);

            s.push({encode, encode, 1, 1});
        } else {
            LRNW lrnw = {UINT64_MAX, 0, 0, 1};

            do {
                LRNW temp = s.top();
                s.pop();

                lrnw.L = std::min(lrnw.L, temp.L);
                lrnw.R = std::max(lrnw.R, temp.R);
                lrnw.N += temp.N;
                lrnw.W += temp.W;

                w -= temp.W;
            } while(w != 0);

            s.push(lrnw);

            if (lrnw.N == lrnw.R - lrnw.L + 1) {
                std::cout << "(" << lrnw.L << "," << lrnw.R << ")" << std::endl;
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
        std::make_pair(4, 2),
        std::make_pair(3, 0),
        std::make_pair(5, 4),
    };

    PSW psw2 {
        std::make_pair(0, 0),
        std::make_pair(3, 0),
        std::make_pair(4, 2),
        std::make_pair(1, 0),
        std::make_pair(5, 4),
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
