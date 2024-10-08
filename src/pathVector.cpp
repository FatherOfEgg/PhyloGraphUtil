#include "pathVector.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

// static uint64_t getRoot(const std::vector<std::vector<uint64_t>> &adjList) {
//     std::vector<uint64_t> inDegree(adjList.size());
//
//     for (const auto &n : adjList) {
//         for (const auto &t : n) {
//             inDegree[t]++;
//         }
//     }
//
//     auto it = std::find(inDegree.begin(), inDegree.end(), 0);
//
//     if (it == inDegree.end()) {
//         std::cerr << "Couldn't find root." << std::endl;
//         std::exit(1);
//     }
//
//     return std::distance(inDegree.begin(), it);
// }

static std::vector<uint64_t> topologicalSort(const std::vector<std::vector<uint64_t>> &adjList) {
    std::vector<uint64_t> inDegree(adjList.size(), 0);

    // Calculate in-degrees
    for (uint64_t u = 0; u < adjList.size(); u++) {
        for (uint64_t v : adjList[u]) {
            inDegree[v]++;
        }
    }

    // Initialize queue with roots (nodes with in-degree 0)
    std::queue<uint64_t> q;
    for (size_t i = 0; i < adjList.size(); i++) {
        if (inDegree[i] == 0) {
            q.push(i);
        }
    }

    std::vector<uint64_t> res;

    while (!q.empty()) {
        uint64_t vertex = q.front();
        q.pop();

        res.push_back(vertex);

        for (uint64_t neighbor : adjList[vertex]) {
            inDegree[neighbor]--;
            if (inDegree[neighbor] == 0) {
                q.push(neighbor);
            }
        }
    }

    return res;
}

static std::vector<std::pair<uint64_t, uint64_t>> pathVectorsHelper(const Graph &g) {
    const uint64_t INF = std::numeric_limits<uint64_t>::max();
    std::vector<uint64_t> shortestPaths(g.adjList.size(), INF);
    std::vector<uint64_t> longestPaths(g.adjList.size(), 0);

    std::vector<uint64_t> topoOrder = topologicalSort(g.adjList);

    std::vector<bool> hasIncomingEdge(g.adjList.size(), false);
    for (uint64_t u = 0; u < g.adjList.size(); u++) {
        for (uint64_t v : g.adjList[u]) {
            hasIncomingEdge[v] = true;
        }
    }

    // Make root node have a distance 0
    for (uint64_t i = 0; i < g.adjList.size(); i++) {
        if (!hasIncomingEdge[i]) {
            shortestPaths[i] = 0;
            longestPaths[i] = 0;
        }
    }

    for (uint64_t vertex : topoOrder) {
        // If this is not a root and we haven't reached it, skip it
        if (shortestPaths[vertex] == INF) {
            continue;
        }

        for (uint64_t child : g.adjList[vertex]) {
            shortestPaths[child] = std::min(shortestPaths[child], shortestPaths[vertex] + 1);
            longestPaths[child] = std::max(longestPaths[child], longestPaths[vertex] + 1);
        }
    }

    std::vector<std::pair<uint64_t, uint64_t>> res;

    for (uint64_t l : g.leaves) {
        res.push_back({shortestPaths[l], longestPaths[l]});
    }

    return res;
}

void findPathVectors(const Graph &g1, const Graph &g2) {
    auto x1 = pathVectorsHelper(g1);

    size_t i = 0;
    for (uint64_t l : g1.leaves) {
        std::cout << g1.leafName.at(l) << " ";
        std::cout << "Shortest: " << x1[i].first << ", ";
        std::cout << "Longest: " << x1[i].second << std::endl;
        i++;
    }

    auto x2 = pathVectorsHelper(g2);
    i = 0;
    for (uint64_t l : g2.leaves) {
        std::cout << g2.leafName.at(l) << " ";
        std::cout << "Shortest: " << x2[i].first << ", ";
        std::cout << "Longest: " << x2[i].second << std::endl;
        i++;
    }
}
