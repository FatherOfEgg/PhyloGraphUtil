#include "psw.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Post order sequence with weights (PSW)
static PSW genPSWHelper(
    const Graph &g,
    const std::vector<std::vector<uint64_t>> &adjList
) {
    PSW res;

    std::function<uint64_t(uint64_t)> postOrder = [&](uint64_t node) -> uint64_t {
        uint64_t weight = 0;

        auto leafIt = g.leafName.find(node);

        if (leafIt == g.leafName.end()) {
            uint64_t childrenWeight = 0;

            for (const uint64_t &c : adjList[node]) {
                childrenWeight += postOrder(c);
            }

            weight += childrenWeight;
        }

        res.push_back(std::make_pair(node, weight));
        return weight + 1;
    };

    postOrder(g.root);

    return res;
}

// Select an edge to use for a reticulation,
// as in, 2 edges go into a node (reticulation),
// and only want to consider one of the edges, not both.
// Also removes unnecessary nodes after choosing an edge,
// for example, A->B->C->D becomes A->D.
static void pruneGraph(
    std::vector<std::vector<uint64_t>> &adjList,
    const std::unordered_map<uint64_t, std::unordered_set<uint64_t>> &parents,
    const std::unordered_map<uint64_t, std::vector<uint64_t>::const_iterator> &curEdges
) {
    // Delete the edge between the reticulation and one of its parents
    for (const auto &p : curEdges) {
        uint64_t curParent = *p.second;

        auto it = std::find(adjList[curParent].begin(), adjList[curParent].end(), p.first);
        adjList[curParent].erase(it);
    }

    for (const auto &p : curEdges) {
        uint64_t curNode = *p.second;
        uint64_t targetNode;
        bool foundTargetNode = false;

        while (!foundTargetNode) {
            size_t s = adjList[curNode].size();

            if (s == 1) {
                targetNode = adjList[curNode][0];
                foundTargetNode = true;
            } else if (s == 0) {
                std::unordered_set<uint64_t> parent = parents.at(curNode);

                for (const uint64_t &e : parent) {
                    if (e != *p.second) {
                        curNode = e;
                        break;
                    }
                }

                auto it = std::find(adjList[curNode].begin(), adjList[curNode].end(), *p.second);
                adjList[curNode].erase(it);
            } else {
                break;
            }
        }

        if (!foundTargetNode) {
            continue;
        }

        // Reduce the edge/node: A->B->C->D becomes A->D
        uint64_t prevNode;

        do {
            prevNode = curNode;
            std::unordered_set<uint64_t> parent = parents.at(curNode);

            if (parent.size() == 2) {
                for (const uint64_t &e : parent) {
                    if (e != *p.second) {
                        curNode = e;
                        break;
                    }
                }
            } else {
                curNode = *parent.begin();
            }
            /* if (parents.at(curNode).size() >= 3) {
                continue;
            } */

        } while (adjList[curNode].size() == 1);

        auto it = std::find(adjList[curNode].begin(), adjList[curNode].end(), prevNode);
        *it = targetNode;
    }
}

static std::unordered_map<uint64_t, std::unordered_set<uint64_t>> getParents(
    const Graph &g
) {
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> res;
    res.reserve(g.adjList.size());

    std::function<void(uint64_t)> postOrder = [&](uint64_t curParent) -> void {
        for (const uint64_t &c : g.adjList[curParent]) {
            res[c].insert(curParent);
            postOrder(c);
        }
    };

    postOrder(g.root);

    return res;
}

std::vector<PSW> genPSWs(
    const Graph &g
) {
    std::vector<std::vector<uint64_t>> copy = g.adjList;
    std::vector<PSW> psw;

    std::unordered_map<uint64_t, std::vector<uint64_t>::const_iterator> curEdges;
    curEdges.reserve(g.reticulations.size());

    for (const auto &p : g.reticulations) {
        curEdges[p.first] = p.second.begin();
    }

    std::vector<uint64_t>::iterator it;

    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> parents = getParents(g);

    while (true) {
        pruneGraph(copy, parents, curEdges);

        psw.emplace_back(genPSWHelper(g, copy));

        auto it = curEdges.begin();
        while (it != curEdges.end()) {
            if (std::next(it->second) != g.reticulations.at(it->first).end()) {
                it->second++;
                break;
            } else {
                it->second = g.reticulations.at(it->first).begin();
            }

            it++;
        }

        if (it == curEdges.end()) {
            break;
        }

        std::copy(g.adjList.begin(), g.adjList.end(), copy.begin());
    }

    return psw;
}
