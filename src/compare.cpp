#include "compare.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

static uint64_t getRoot(const std::vector<std::vector<uint64_t>> &adjList) {
    std::vector<uint64_t> inDegree(adjList.size());

    for (const auto &n : adjList) {
        for (const auto &t : n) {
            inDegree[t]++;
        }
    }

    auto it = std::find(inDegree.begin(), inDegree.end(), 0);

    if (it == inDegree.end()) {
        std::cerr << "Couldn't find root." << std::endl;
        std::exit(1);
    }

    return std::distance(inDegree.begin(), it);
}

static std::unordered_set<std::string> dfs(
    uint64_t node,
    uint64_t rootNode,
    const std::vector<std::vector<uint64_t>> &adjList,
    const std::unordered_map<uint64_t, std::string> &leafName,
    std::vector<std::unordered_set<std::string>> &subtreeLeaves,
    std::vector<std::unordered_set<std::string>> &splits
) {
    if (leafName.find(node) != leafName.end()) {
        // Leaf is more or less a subtree of itself
        subtreeLeaves[node].insert(leafName.at(node));
        return subtreeLeaves[node];
    }

    for (const uint64_t &c : adjList[node]) {
        // Get this node's children's subtree and the leaves that they each can reach
        std::unordered_set<std::string> children = dfs(c, rootNode, adjList, leafName, subtreeLeaves, splits);
        subtreeLeaves[node].insert(children.begin(), children.end());
    }

    if (node != rootNode) {
        splits.push_back(subtreeLeaves[node]);
    }

    // Return the leaves that this node can reach
    return subtreeLeaves[node];
}

static std::vector<std::unordered_set<std::string>> getSplits(
    const std::vector<std::vector<uint64_t>> &adjList,
    const std::unordered_map<uint64_t, std::string> &leafName
) {
    uint64_t root = getRoot(adjList);

    std::vector<std::unordered_set<std::string>> splits;
    std::vector<std::unordered_set<std::string>> subtreeLeaves(adjList.size());

    dfs(root, root, adjList, leafName, subtreeLeaves, splits);
    return splits;
}

static uint64_t rf_dist(
    const std::vector<std::vector<uint64_t>> &adjList1, const std::unordered_map<uint64_t, std::string> &leafName1,
    const std::vector<std::vector<uint64_t>> &adjList2, const std::unordered_map<uint64_t, std::string> &leafName2
) {
    std::vector<std::unordered_set<std::string>> splits1 = getSplits(adjList1, leafName1);
    std::vector<std::unordered_set<std::string>> splits2 = getSplits(adjList2, leafName2);

    // TODO: Figure out a better way to do this, maybe bit manip?
    auto concatSplit = [](const std::unordered_set<std::string> &split) {
        std::string res;

        for (const auto &leaf : split) {
            res += leaf + ",";
        }

        return res;
    };

    std::unordered_set<std::string> s1;
    for (const auto &s : splits1) {
        s1.insert(concatSplit(s));
    }

    std::unordered_set<std::string> s2;
    for (const auto &s : splits2) {
        s2.insert(concatSplit(s));
    }

    uint64_t commonSplits = 0;
    for (const auto& split : s1) {
        if (s2.find(split) != s2.end()) {
            commonSplits++;
        }
    }

    uint64_t rf_distance = s1.size() + s2.size() - 2 * commonSplits;
    return rf_distance;
}

void compare(const Graph &g1, const Graph &g2) {
    if (g1.leaves.size() != g2.leaves.size()) {
        std::cerr << "Trees do not have the same number of leaves." << std::endl;
        std::exit(1);
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
        std::exit(1);
    }

    if (g1.reticulations.size() != g2.reticulations.size()) {
        std::cerr << "Trees do not have the same number of reticulations." << std::endl;
        std::exit(1);
    }

    std::vector<uint64_t> dists;

    uint64_t dist = rf_dist(g1.adjList, g1.leafName, g2.adjList, g2.leafName);
    dists.push_back(dist);

    uint64_t smallestDist = *std::min_element(dists.begin(), dists.end());
    size_t n = g1.leaves.size();
    double percentage = static_cast<double>(smallestDist) / 2 * (n - 3) * 100;

    std::cout << percentage << "% different (smallest RF dist: " << smallestDist << ")" << std::endl;
}
