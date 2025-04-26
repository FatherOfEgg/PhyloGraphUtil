#include "precisionAndRecall.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "util/clusterTable.h"
#include "util/psw.h"

using ClusterMultiset = std::unordered_map<uint64_t, std::unordered_multiset<uint64_t>>;
using ClusterSet = std::unordered_map<uint64_t, std::unordered_set<uint64_t>>;

static ClusterMultiset extractClusters(
    const Graph &g, const std::vector<PSW> &psws
) {
    ClusterMultiset res;

    for (const PSW &psw : psws) {
        ClusterTable ct(g, psw);

        for (const auto &l : ct.ct) {
            res[l.first].insert(l.second.begin(), l.second.end());
        }
    }

    return res;
}

// COMCLUST
template <typename CT>
static double calculateIntersection(
    const CT &ct,
    uint64_t maxCount,
    const Graph &g2, const std::vector<PSW> &psws2,
    std::unordered_map<std::string, uint64_t> internalLabels
) {
    double res = 0.0;
    std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t>> count;

    std::stack<LRNW> s;

    for (const PSW &psw2 : psws2) {
        for (size_t i = 0; i < psw2.size(); i++) {
            const std::pair<uint64_t, uint64_t> p = psw2[i];

            uint64_t w = p.second;

            // If leaf
            if (w == 0) {
                std::string leafName = g2.leafName.at(p.first);
                uint64_t encode = internalLabels.at(leafName);

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
                    auto itL = ct.find(lrnw.L);

                    if (itL == ct.end()) {
                        continue;
                    }

                    auto itR = itL->second.find(lrnw.R);

                    if (itR != itL->second.end()) {
                        if (count[lrnw.L][lrnw.R] < maxCount) {
                            count[lrnw.L][lrnw.R]++;
                            res++;
                        }
                    }
                }
            }
        }
    }

    return res;
}

template <typename T1, typename T2>
static std::pair<double, double> calculatePNR(
    const T1 &original, const T2 &compare,
    double intersection
) {
    double originalSize = 0.0;
    for (const auto &l : original) {
        originalSize += l.second.size();
    }

    double compareSize = 0.0;
    for (const auto &l : compare) {
        compareSize += l.second.size();
    }

    double precision = 0.0;

    if (compareSize > 0.0) {
        precision = intersection / compareSize;
    }

    double recall = 0.0;

    if (originalSize > 0.0) {
        recall = intersection / originalSize;
    }

    return std::make_pair(precision, recall);
}

static double calculateF1Score(double precision, double recall) {
    return 2.0 * (precision * recall) / (precision + recall);
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

    std::unordered_map<std::string, uint64_t> internalLabels;
    {
        ClusterTable ct(g1, psws1[0]);
        internalLabels = std::move(ct.internalLabels);
    }

    ClusterMultiset originalDup = extractClusters(g1, psws1);
    ClusterMultiset compareDup = extractClusters(g2, psws2);

    uint64_t subtrees1 = std::pow(2, g1.reticulations.size());
    uint64_t subtrees2 = std::pow(2, g2.reticulations.size());
    uint64_t maxCount = std::min(subtrees1, subtrees2);
    double intersectionDup = calculateIntersection(originalDup, maxCount, g2, psws2, internalLabels);

    ClusterSet originalUniq(originalDup.size());
    for (const auto &l : originalDup) {
        originalUniq[l.first].insert(l.second.begin(), l.second.end());
    }

    ClusterSet compareUniq(compareDup.size());
    for (const auto &l : compareDup) {
        compareUniq[l.first].insert(l.second.begin(), l.second.end());
    }

    double intersectionUniq = calculateIntersection(originalUniq, 1, g2, psws2, internalLabels);

    std::cout << "Duplicates:" << std::endl;
    std::pair<double, double> pnrDup = calculatePNR(originalDup, compareDup, intersectionDup);
    double f1scoreDup = calculateF1Score(pnrDup.first, pnrDup.second);

    std::cout << "Precision: " << pnrDup.first << std::endl;
    std::cout << "Recall: " << pnrDup.second << std::endl;
    std::cout << "F1 score: " << f1scoreDup << std::endl;

    std::cout << std::endl;

    std::cout << "Unique:" << std::endl;
    std::pair<double, double> pnrUniq = calculatePNR(originalUniq, compareUniq, intersectionUniq);
    double f1scoreUniq = calculateF1Score(pnrUniq.first, pnrUniq.second);

    std::cout << "Precision: " << pnrUniq.first << std::endl;
    std::cout << "Recall: " << pnrUniq.second << std::endl;
    std::cout << "F1 score: " << f1scoreUniq << std::endl;
}
