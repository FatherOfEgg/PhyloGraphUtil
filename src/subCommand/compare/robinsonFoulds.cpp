#include "robinsonFoulds.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
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
// Returns a pair (dissimilarity, similarity)
static std::pair<uint64_t, uint64_t> rfDist(
    const ClusterTable &ct1,
    const ClusterTable &ct2,
    const Graph &g2, const PSW &psw2
) {
    uint64_t commonClusters = 0;

    std::stack<LRNW> s;

    for (size_t i = 0; i < psw2.size(); i++) {
        const std::pair<uint64_t, uint64_t> p = psw2[i];

        uint64_t w = p.second;

        // If leaf
        if (w == 0) {
            std::string leafName = g2.leafName.at(p.first);
            uint64_t encode = ct1.encode(leafName);

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

            if (lrnw.N == lrnw.R - lrnw.L + 1
            && ct1.isClust(lrnw.L, lrnw.R)) {
                commonClusters++;
            }
        }
    }

    uint64_t dissimilarity = ct1.size + ct2.size - 2 * commonClusters;
    return std::make_pair(dissimilarity, commonClusters);
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

    /* if (g1.reticulations.size() != g2.reticulations.size()) {
        std::cerr << "Trees do not have the same number of reticulations." << std::endl;
        std::exit(EXIT_FAILURE);
    } */

    std::vector<PSW> psws1 = genPSWs(g1);
    std::vector<PSW> psws2 = genPSWs(g2);

    size_t n = psws1.size();
    size_t m = psws2.size();
    size_t size = std::max(n, m);

    std::vector<ClusterTable> cts1;
    cts1.reserve(n);
    std::vector<ClusterTable> cts2;
    cts2.reserve(m);

    for (size_t i = 0; i < n; i++) {
        cts1.emplace_back(g1, psws1[i]);
    }

    for (size_t i = 0; i < m; i++) {
        cts2.emplace_back(g2, psws2[i]);
    }

    std::vector<std::vector<cost>> costMatrix(size, std::vector<cost>(size, 0.0));

    uint64_t sumMinDist = 0;
    for (size_t i = 0; i < n; i++) {
        uint64_t minDist = UINT64_MAX;

        for (size_t j = 0; j < m; j++) {
            auto p = rfDist(cts1[i], cts2[j], g2, psws2[j]);
            uint64_t dist = p.first;

            costMatrix[i][j] = static_cast<cost>(dist);

            if (dist < minDist) {
                minDist = dist;
            }
        }

        sumMinDist += minDist;
    }

    /* for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < m; j++) {
            std::cout << costMatrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::exit(0); */

    std::vector<col> rowSol(size);
    std::vector<row> colSol(size);
    std::vector<cost> u(size);
    std::vector<cost> v(size);

    cost c = lap(size, costMatrix, rowSol, colSol, u, v);

    size_t numLeaves = g1.leaves.size();
    double maxRFDist = (2 * (numLeaves - 2)) * size;

    std::cout << "Bipartite matching:" << std::endl;
    std::cout << c / maxRFDist * 100.0 << "% difference " << std::endl;
    std::cout << "Total RF dist: " << c << std::endl;

    std::cout << std::endl;

    std::cout << "Sum of the smallest RF distances:" << std::endl;
    std::cout << (static_cast<double>(sumMinDist) / maxRFDist) * 100.0 << "% difference" << std::endl;
    std::cout << "Total RF dist: " << sumMinDist << std::endl;

    std::cout << std::endl;

    std::cout << "Non-trivial splits:" << std::endl;
    std::cout << "W/o considering every contained subtree: " << maxRFDist / size << std::endl;
    std::cout << "Considering every contained subtree: " << maxRFDist << std::endl;
}
