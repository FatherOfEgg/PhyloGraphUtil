#include "robinsonFoulds.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
// #include <iomanip>
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

static void printBipartiteStats(
    const std::vector<std::vector<cost>> &costMatrix,
    const std::vector<std::vector<uint64_t>> &similarity,
    const std::vector<col> &rowSol
) {
    uint64_t min = costMatrix[0][rowSol[0]];
    uint64_t max = costMatrix[0][rowSol[0]];
    double total = costMatrix[0][rowSol[0]];
    double similarityTotal = similarity[0][rowSol[0]];

    for (size_t i = 1; i < rowSol.size(); i++) {
        cost dissimilarity = costMatrix[i][rowSol[i]];

        if (dissimilarity < min) {
            min = dissimilarity;
        }
        if (dissimilarity > max) {
            max = dissimilarity;
        }

        total += dissimilarity;
        similarityTotal += similarity[i][rowSol[i]];
    }

    std::cout << total / (total + similarityTotal) * 100.0 << "% difference" << std::endl;

    std::cout << "RF Distance" << std::endl;
    std::cout << "total: " << total;
    std::cout << ", min: " << min;
    std::cout << ", max: " << max;
    std::cout << ", avg: " << total / rowSol.size() << std::endl;

    /* int width1 = 5;
    int width2 = 7;

    std::cout << "RF Distance" << std::endl;

    std::cout << std::right;
    std::cout << std::setw(width1) << "min";
    std::cout << std::setw(width1) << "max";
    std::cout << std::setw(width1) << "avg";
    std::cout << std::setw(width2) << "total" << std::endl;

    std::cout << std::setfill('-');
    std::cout << std::setw(width1) << "";
    std::cout << std::setw(width1) << "";
    std::cout << std::setw(width1) << "";
    std::cout << std::setw(width2) << "" << std::endl;

    std::cout << std::setfill(' ');
    std::cout << std::setw(width1) << min;
    std::cout << std::setw(width1) << max;
    std::cout << std::setw(width1) << total / rowSol.size();
    std::cout << std::setw(width2) << total << std::endl; */
}

static void printSmallestSumStats(
    const std::vector<uint64_t> &minDissimilarity,
    const std::vector<uint64_t> &minSimilarity
) {
    uint64_t min = minDissimilarity[0];
    uint64_t max = minDissimilarity[0];
    double total = minDissimilarity[0];
    double similarityTotal = minSimilarity[0];

    for (size_t i = 1; i < minDissimilarity.size(); i++) {
        cost dissimilarity = minDissimilarity[i];

        if (dissimilarity < min) {
            min = dissimilarity;
        }
        if (dissimilarity > max) {
            max = dissimilarity;
        }

        total += dissimilarity;
        similarityTotal += minSimilarity[i];
    }

    std::cout << total / (total + similarityTotal) * 100.0 << "% difference" << std::endl;

    std::cout << "RF Distance" << std::endl;
    std::cout << "total: " << total;
    std::cout << ", min: " << min;
    std::cout << ", max: " << max;
    std::cout << ", avg: " << total / minDissimilarity.size() << std::endl;

}

static void printClusterStats(
    const Graph &g,
    std::vector<ClusterTable> cts
) {
    uint64_t minCluster = cts[0].size;
    double sumCluster = cts[0].size;
    uint64_t maxCluster = cts[0].size;

    for (size_t i = 1; i < cts.size(); i++) {
        uint64_t s = cts[i].size;

        if (s < minCluster) {
            minCluster = s;
        }
        if (s > maxCluster) {
            maxCluster = s;
        }
        sumCluster += s;
    }

    std::cout << g.filename << ":" << std::endl;

    std::cout << "total (considering every contained subtree): " << sumCluster << std::endl;
    std::cout << "min: " << minCluster;
    std::cout << ", max: " << maxCluster;
    std::cout << ", avg: " << sumCluster / cts.size() << std::endl;
}

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
    std::vector<std::vector<uint64_t>> similarity(n, std::vector<uint64_t>(m));

    std::vector<uint64_t> minDissimilarity;
    minDissimilarity.reserve(n);
    std::vector<uint64_t> minSimilarity;
    minSimilarity.reserve(n);

    for (size_t i = 0; i < n; i++) {
        uint64_t minDist = UINT64_MAX;
        uint64_t minSim = 0;

        for (size_t j = 0; j < m; j++) {
            auto p = rfDist(cts1[i], cts2[j], g2, psws2[j]);
            uint64_t dist = p.first;
            uint64_t sim = p.second;

            costMatrix[i][j] = static_cast<cost>(dist);
            similarity[i][j] = sim;

            if (dist < minDist) {
                minDist = dist;
                minSim = sim;
            }
        }

        minDissimilarity.push_back(minDist);
        minSimilarity.push_back(minSim);
    }

    std::vector<col> rowSol(size);
    std::vector<row> colSol(size);
    std::vector<cost> u(size);
    std::vector<cost> v(size);

    lap(size, costMatrix, rowSol, colSol, u, v);

    std::cout << "===Bipartite matching===" << std::endl;
    printBipartiteStats(costMatrix, similarity, rowSol);
    std::cout << std::endl;

    std::cout << "===Sum of the smallest===" << std::endl;
    printSmallestSumStats(minDissimilarity, minSimilarity);
    std::cout << std::endl;

    std::cout << "===Non-trivial clades/clusters===" << std::endl;
    printClusterStats(g1, cts1);
    std::cout << std::endl;
    printClusterStats(g2, cts2);
}
