#include "cluster.h"

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

#include "bitmask.h"

static BitmaskSet computeClustersHelper(
    const Graph &g,
    const std::unordered_map<std::string, uint64_t> &bitmaskId,
    const std::unordered_map<uint64_t, uint8_t> &curEdges
) {
    BitmaskSet res;

    std::function<Bitmask(uint64_t)> postOrder = [&](uint64_t node) -> Bitmask {
        Bitmask bm;

        auto leafIt = g.leafName.find(node);

        if (leafIt != g.leafName.end()) {
            uint64_t id = bitmaskId.at(leafIt->second);
            bm.setBit(1 << id);
        } else {
            for (const uint64_t &c : g.adjList[node]) {
                auto it = g.reticulations.find(c);

                // Check if the current node has an edge
                // to a reticulation at child c
                if (it != g.reticulations.end()) {
                    // Child c is a reticulation, so get its parents
                    const std::vector<uint64_t> &parents = it->second;
                    uint8_t curEdge = curEdges.at(c);

                    // curEdge indexes into parents
                    // Continue if there is "no" edge from node to reticulation
                    //
                    if (parents[curEdge] != node) {
                        continue;
                    }
                }

                bm |= postOrder(c);
            }
        }

        res.insert(bm);
        return bm;
    };

    postOrder(g.root);

    return res;
}

std::vector<BitmaskSet> computeClusters(
    const Graph &g,
    const std::unordered_map<std::string, uint64_t> &bitmaskId
) {
    std::vector<BitmaskSet> clusters;

    std::unordered_map<uint64_t, uint8_t> curEdges;
    curEdges.reserve(g.reticulations.size());

    for (const auto &p : g.reticulations) {
        curEdges[p.first] = 0;
    }

    while (true) {
        clusters.push_back(computeClustersHelper(g, bitmaskId, curEdges));

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

    return clusters;
}
