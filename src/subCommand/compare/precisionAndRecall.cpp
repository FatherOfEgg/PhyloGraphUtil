#include "precisionAndRecall.h"

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
static std::pair<double, double> calculatePNR(
    CT originalCT, const Graph &originalG, const std::vector<PSW> &originalPSWs,
    const std::unordered_map<std::string, uint64_t> &originalInternalLabels,
    const CT &compareCT, const Graph &compareG, const std::vector<PSW> &comparePSWs,
    const std::unordered_map<std::string, uint64_t> &compareInternalLabels
) {
    uint64_t truePositives = 0;
    uint64_t falsePositives = 0;

    for (const PSW &psw : comparePSWs) {
        std::stack<LRNW> s;

        for (size_t i = 0; i < psw.size(); i++) {
            const std::pair<uint64_t, uint64_t> p = psw[i];

            uint64_t w = p.second;

            // If leaf
            if (w == 0) {
                std::string leafName = compareG.leafName.at(p.first);
                uint64_t encode = originalInternalLabels.at(leafName);

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
                    auto itL = originalCT.find(lrnw.L);

                    if (itL == originalCT.end()) {
                        falsePositives++;
                        continue;
                    }

                    auto itR = itL->second.find(lrnw.R);

                    if (itR != itL->second.end()) {
                        itL->second.erase(itR);
                        truePositives++;
                    } else {
                        falsePositives++;
                    }

                    if (itL->second.empty()) {
                        originalCT.erase(itL);
                    }
                }
            }
        }
    }

    for (const auto &l : originalCT) {
        falsePositives += l.second.size();
    }

    uint64_t falseNegatives = 0;
    for (const auto &l : compareCT) {
        falseNegatives += l.second.size();
    }

    for (const PSW &psw : originalPSWs) {
        std::stack<LRNW> s;

        for (size_t i = 0; i < psw.size(); i++) {
            const std::pair<uint64_t, uint64_t> p = psw[i];

            uint64_t w = p.second;

            // If leaf
            if (w == 0) {
                std::string leafName = originalG.leafName.at(p.first);
                uint64_t encode = compareInternalLabels.at(leafName);

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
                    auto itL = compareCT.find(lrnw.L);

                    if (itL == compareCT.end()) {
                        continue;
                    }

                    auto itR = itL->second.find(lrnw.R);

                    if (itR != itL->second.end()) {
                        if (falseNegatives > 0) {
                            falseNegatives--;
                        }
                    }
                }
            }
        }
    }

    double precision = 0.0;
    double predictedPositives = truePositives + falsePositives;
    if (predictedPositives > 0.0) {
        precision = static_cast<double>(truePositives) / predictedPositives;
    }

    double recall = 0.0;
    double actualPositives = truePositives + falseNegatives;
    if (actualPositives > 0.0) {
        recall = static_cast<double>(truePositives) / actualPositives;
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

    std::unordered_map<std::string, uint64_t> originalInternalLabels;
    {
        ClusterTable ct(g1, psws1[0]);
        originalInternalLabels = std::move(ct.internalLabels);
    }

    std::unordered_map<std::string, uint64_t> compareInternalLabels;
    {
        ClusterTable ct(g2, psws2[0]);
        compareInternalLabels = std::move(ct.internalLabels);
    }

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

    std::cout << "Duplicates clusters:" << std::endl;
    std::pair<double, double> pnrDup = calculatePNR(
        originalDup, g1, psws1,
        originalInternalLabels,
        compareDup, g2, psws2,
        compareInternalLabels
    );

    double f1scoreDup = calculateF1Score(pnrDup.first, pnrDup.second);

    std::cout << "Precision: " << pnrDup.first << std::endl;
    std::cout << "Recall: " << pnrDup.second << std::endl;
    std::cout << "F1 score: " << f1scoreDup << std::endl;

    std::cout << std::endl;

    std::cout << "Unique clusters:" << std::endl;
    std::pair<double, double> pnrUniq = calculatePNR(
        originalUniq, g1, psws1,
        originalInternalLabels,
        compareUniq, g2, psws2,
        compareInternalLabels
    );
    double f1scoreUniq = calculateF1Score(pnrUniq.first, pnrUniq.second);

    std::cout << "Precision: " << pnrUniq.first << std::endl;
    std::cout << "Recall: " << pnrUniq.second << std::endl;
    std::cout << "F1 score: " << f1scoreUniq << std::endl;
}
