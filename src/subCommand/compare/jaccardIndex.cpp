#include "jaccardIndex.h"

#include <iostream>

#include "util/cluster.h"

void jaccardIndex(const Graph &g1, const Graph &g2) {
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
    BitmaskSet bs1;
    BitmaskSet bs2;

    for (const BitmaskSet &bs : c1) {
        bs1.insert(bs.begin(), bs.end());
    }

    for (const BitmaskSet &bs : c2) {
        bs2.insert(bs.begin(), bs.end());
    }

    uint64_t intersection = 0;

    for (const Bitmask &bs : bs1) {
        if (bs2.find(bs) != bs2.end()) {
            intersection++;
        }
    }

    // Union
    bs1.insert(bs2.begin(), bs2.end());

    double similarity = static_cast<double>(intersection) / bs1.size();
    double diff = 1 - similarity;
    double percentage = diff * 100.0;

    std::cout << percentage << "% different (" << diff << ")" << std::endl;
}
