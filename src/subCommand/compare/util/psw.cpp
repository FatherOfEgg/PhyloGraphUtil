#include "psw.h"

#include <cstdint>
#include <functional>
#include <vector>

// Post order sequence with weights (PSW)
static PSW genPSWHelper(
    const Graph &g,
    const std::unordered_map<uint64_t, uint8_t> &curEdges
) {
    PSW res;

    std::function<uint64_t(uint64_t)> postOrder = [&](uint64_t node) -> uint64_t {
        uint64_t weight = 0;

        auto leafIt = g.leafName.find(node);

        if (leafIt == g.leafName.end()) {
            uint64_t childrenWeight = 0;
            bool ignoreNode = false;

            for (const uint64_t &c : g.adjList[node]) {
                auto it = g.reticulations.find(c);

                // Check if the current node has an edge
                // to a reticulation at child c
                if (it != g.reticulations.end()) {
                    // Child c is a reticulation, so get its parents
                    const std::vector<uint64_t> &parents = it->second;
                    uint8_t curEdge = curEdges.at(c);

                    // Continue if there's "no" edge from reticulation's parent
                    // to the reticulation c
                    if (parents[curEdge] != node) {
                        ignoreNode = true;
                        continue;
                    }
                }

                childrenWeight += postOrder(c);
            }

            if (ignoreNode) {
                // If this node has an edge to a reticulation,
                // but currently doesn't "have" a connection to it currently,
                // then we ignore this node and don't consider it in the PSW.
                //
                // For example, if we have: ((1, (2)#H1), (#H1, 3)),
                // and we extract a subtree from it, it would be:
                // ((1, 2), (3)). However, we don't want it as "(3)",
                // instead we want the subree to be: ((1, 2), 3)
                return childrenWeight;
            } else {
                weight += childrenWeight;
            }
        }

        res.push_back(std::make_pair(node, weight));
        return weight + 1;
    };

    postOrder(g.root);

    return res;
}

std::vector<PSW> genPSWs(
    const Graph &g
) {
    std::vector<PSW> psw;

    std::unordered_map<uint64_t, uint8_t> curEdges;
    curEdges.reserve(g.reticulations.size());

    for (const auto &p : g.reticulations) {
        curEdges[p.first] = 0;
    }

    while (true) {
        psw.emplace_back(genPSWHelper(g, curEdges));

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

    return psw;
}
