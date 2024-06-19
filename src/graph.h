#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "attribute.h"

struct Node {
    std::string id;
    std::string label;
    AttributeMap attributes;
};

struct Edge {
    std::string source;
    std::string target;
    std::string label;
    double weight;
    double length;
};

class Graph {
public:
    Graph();
    Graph(const Graph &other);
    virtual ~Graph();

    Graph &operator=(const Graph &other);

    virtual void addNode(Node &n);
    virtual void addNode(const std::string &id, const std::string &label, AttributeMap &attributes);
    unsigned int getNumNodes() const;

    virtual void addEdge(Edge &e);
    virtual void addEdge(const std::string &source, const std::string &target, const std::string &label, double weight, double length);
    unsigned int getNumEdges() const;

    void addAttribute(const std::string &s, std::unique_ptr<Attribute> value);

    virtual void open(const std::string &file) = 0;
    virtual void save(const std::string &filename) const = 0;

    virtual void print() const = 0;

protected:
    std::vector<Node> mNodes;
    std::unordered_map<std::string, std::vector<Edge>> mEdges;
    std::unordered_map<std::string, size_t> mIdToIndex;
    AttributeMap mAttributes;
};
