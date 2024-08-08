#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct Node {
    std::string id;
    std::string label;
};

struct Edge {
    std::string source;
    std::string target;
    std::string label;
    double length;
};

struct Graph {
public:
    void addNode(Node &n);
    void addNode(const std::string &id, const std::string &label);
    unsigned int getNumNodes() const;

    void addEdge(Edge &e);
    void addEdge(const std::string &source, const std::string &target, const std::string &label, double length);
    unsigned int getNumEdges() const;

    void print() const;

public:
    std::vector<Node> nodes;
    std::unordered_map<std::string, std::vector<Edge>> edges;
    std::unordered_map<std::string, size_t> idToIndex;
};
