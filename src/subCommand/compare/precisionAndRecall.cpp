#include "precisionAndRecall.h"

#include <cstdint>
#include <iostream>
#include <unordered_set>
#include <utility>
#include <vector>

#include "util/clusterTable.h"
#include "util/psw.h"

using ClusterMultiset = std::unordered_map<uint64_t, std::unordered_multiset<uint64_t>>;
using ClusterSet = std::unordered_map<uint64_t, std::unordered_set<uint64_t>>;

template <typename T1, typename T2>
static std::pair<double, double> calculatePNR(const T1 &original, const T2 &compare) {
    uint64_t intersection = 0;

    for (const auto &l : original) {
        auto it = compare.find(l.first);

        if (it == compare.end()) {
            continue;
        }

        for (const auto &r : l.second) {
            uint64_t originalCount = l.second.count(r);
            uint64_t compareCount = it->second.count(r);

            if (compareCount > 0) {
                intersection += std::min(originalCount, compareCount);
            }
        }
    }

    uint64_t originalSize = 0;
    for (const auto &e : original) {
        originalSize += e.second.size();
    }

    uint64_t compareSize = 0;
    for (const auto &e : compare) {
        compareSize += e.second.size();
    }

    double precision = 0.0;

    if (compareSize > 0.0) {
        precision = static_cast<double>(intersection) / compareSize;
    }

    double recall = 0.0;

    if (originalSize > 0.0) {
        recall = static_cast<double>(intersection) / originalSize;
    }

    return std::make_pair(precision, recall);
}

static double calculateF1Score(double precision, double recall) {
    return 2.0 * (precision * recall) / (precision + recall);
}

static ClusterMultiset extractClusters(const Graph &g, const std::vector<PSW> &psws) {
    ClusterMultiset res;

    for (const PSW &psw : psws) {
        ClusterTable ct(g, psw);

        for (const auto &l : ct.ct) {
            res[l.first].insert(l.second.begin(), l.second.end());
        }
    }

    return res;
}

// Graph g1 is the original
void precisionAndRecall(const Graph &g1, const Graph &g2) {
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

    std::vector<PSW> psws1 = genPSWs(g1);
    std::vector<PSW> psws2 = genPSWs(g2);

    ClusterMultiset originalDup = extractClusters(g1, psws1);
    ClusterMultiset compareDup = extractClusters(g2, psws2);

    ClusterSet originalUniq(originalDup.size());
    for (const auto &l : originalDup) {
        originalUniq[l.first].insert(l.second.begin(), l.second.end());
    }

    ClusterSet compareUniq(compareDup.size());
    for (const auto &l : compareDup) {
        compareUniq[l.first].insert(l.second.begin(), l.second.end());
    }

    std::cout << "Duplicates:" << std::endl;
    std::pair<double, double> pnrDup = calculatePNR(originalDup, compareDup);
    double f1scoreDup = calculateF1Score(pnrDup.first, pnrDup.second);

    std::cout << "Precision: " << pnrDup.first << std::endl;
    std::cout << "Recall: " << pnrDup.second << std::endl;
    std::cout << "F1 score: " << f1scoreDup << std::endl;

    std::cout << std::endl;

    std::cout << "Unique:" << std::endl;
    std::pair<double, double> pnrUniq = calculatePNR(originalUniq, compareUniq);
    double f1scoreUniq = calculateF1Score(pnrUniq.first, pnrUniq.second);

    std::cout << "Precision: " << pnrUniq.first << std::endl;
    std::cout << "Recall: " << pnrUniq.second << std::endl;
    std::cout << "F1 score: " << f1scoreUniq << std::endl;
}
