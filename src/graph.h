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
    std::unordered_map<std::string, std::shared_ptr<Attribute>> attributes;
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
    virtual ~Graph();

    virtual void addNode(Node &n);
    virtual void addNode(const std::string &id, const std::string &label, std::unordered_map<std::string, std::shared_ptr<Attribute>> &attributes);
    unsigned int getNumNodes() const;

    virtual void addEdge(Edge &e);
    virtual void addEdge(const std::string &source, const std::string &target, const std::string &label, double weight, double length);
    unsigned int getNumEdges() const;

    virtual void open(const std::string &file) = 0;
    virtual void print() const = 0;

public:
    std::vector<Node> nodes;
    std::unordered_map<std::string, std::vector<Edge>> edges;
    std::unordered_map<std::string, size_t> idToIndex;
    std::unordered_map<std::string, std::shared_ptr<Attribute>> attributes;
};
