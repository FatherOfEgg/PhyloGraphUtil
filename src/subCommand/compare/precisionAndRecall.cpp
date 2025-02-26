#include "precisionAndRecall.h"

#include <iostream>
#include <unordered_set>
#include <utility>

#include "util/bitmask.h"
#include "util/cluster.h"

using BitmaskSet = std::unordered_set<Bitmask, BitmaskHash>;
using BitmaskMultiset = std::unordered_multiset<Bitmask, BitmaskHash>;

template <typename T1, typename T2>
static std::pair<double, double> calculatePNR(const T1 &original, const T2 &compare) {
    uint64_t intersection = 0;

    for (const auto &e : original) {
        if (compare.find(e) != compare.end()) {
            intersection++;
        }
    }

    double precision = 0.0;

    if (compare.size() > 0.0) {
        precision = static_cast<double>(intersection) / compare.size();
    }

    double recall = 0.0;

    if (original.size() > 0.0) {
        recall = static_cast<double>(intersection) / original.size();
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

    // Setup bitmask ids
    std::unordered_map<std::string, uint64_t> bitmaskId;

    leaves1.insert(leaves2.begin(), leaves2.end());

    for (const std::string &l : leaves1) {
        bitmaskId[l] = bitmaskId.size();
    }

    std::vector<BitmaskSet> c1 = computeClusters(g1, bitmaskId);
    std::vector<BitmaskSet> c2 = computeClusters(g2, bitmaskId);

    // Merge all the bitmask sets into 1 set
    // TODO: Find a different way so that we don't have to
    // merge them later and instead just insert into a set.
    // Probably modify computeClusters
    BitmaskMultiset originalDup;
    BitmaskMultiset compareDup;

    for (const BitmaskSet &bs : c1) {
        originalDup.insert(bs.begin(), bs.end());
    }

    for (const BitmaskSet &bs : c2) {
        compareDup.insert(bs.begin(), bs.end());
    }

    BitmaskSet originalUniq(originalDup.begin(), originalDup.end());
    BitmaskSet compareUniq(compareDup.begin(), compareDup.end());

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
