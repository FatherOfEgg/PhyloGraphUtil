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

class Graph {
public:
    Graph();
    Graph(const Graph &other);
    virtual ~Graph();

    Graph &operator=(const Graph &other);

    virtual void addNode(Node &n);
    virtual void addNode(const std::string &id, const std::string &label);
    unsigned int getNumNodes() const;

    virtual void addEdge(Edge &e);
    virtual void addEdge(const std::string &source, const std::string &target, const std::string &label, double length);
    unsigned int getNumEdges() const;

    virtual void open(const std::string &file) = 0;
    virtual void save(const std::string &filename) const = 0;

    virtual void print() const;

protected:
    std::vector<Node> mNodes;
    std::unordered_map<std::string, std::vector<Edge>> mEdges;
    std::unordered_map<std::string, size_t> mIdToIndex;
};
