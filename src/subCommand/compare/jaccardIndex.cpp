#include "jaccardIndex.h"

#include <iostream>

#include "util/clusterTable.h"
#include "util/psw.h"

using ClusterSet = std::unordered_map<uint64_t, std::unordered_set<uint64_t>>;

static ClusterSet extractClusters(const Graph &g, const std::vector<PSW> &psws) {
    ClusterSet res;

    for (const PSW &psw : psws) {
        ClusterTable ct(g, psw);

        for (const auto &l : ct.ct) {
            res[l.first].insert(l.second.begin(), l.second.end());
        }
    }

    return res;
}

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

    std::vector<PSW> psws1 = genPSWs(g1);
    std::vector<PSW> psws2 = genPSWs(g2);

    ClusterSet c1 = extractClusters(g1, psws1);
    ClusterSet c2 = extractClusters(g2, psws2);

    double intersection = 0.0;

    for (const auto &p : c1) {
        if (c2.find(p.first) == c2.end()) {
            continue;
        }

        for (const auto &e : p.second) {
            if (c2[p.first].find(e) != c2[p.first].end()) {
                intersection++;
            }
        }
    }

    // Union
    for (const auto &p : c2) {
        c1[p.first].insert(p.second.begin(), p.second.end());
    }

    double unionSize = 0.0;

    for (const auto &p : c1) {
        unionSize += p.second.size();
    }

    double similarity = intersection / unionSize;
    double diff = 1 - similarity;
    double percentage = diff * 100.0;

    std::cout << percentage << "% different (" << diff << ")" << std::endl;
}
