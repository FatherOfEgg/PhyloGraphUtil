#include "graph.h"
#include "attribute.h"

Graph::Graph() {}

Graph::Graph(const Graph &other)
: mNodes(other.mNodes), mEdges(other.mEdges),
  mIdToIndex(other.mIdToIndex),
  mAttributes(other.mAttributes) {}

Graph::~Graph() {}

Graph &Graph::operator=(const Graph &other) {
    if (this != &other) {
        mNodes.clear();
        mEdges.clear();
        mIdToIndex.clear();
        mAttributes.clear();

        mNodes = other.mNodes;
        mEdges = other.mEdges;
        mIdToIndex = other.mIdToIndex;
        mAttributes = other.mAttributes;
    }

    return *this;
}

void Graph::addNode(Node &n) {
    mNodes.push_back(std::move(n));
    mIdToIndex[n.id] = mNodes.size() - 1;
}

void Graph::addNode(const std::string &id, const std::string &label, AttributeMap &attributes) {
    mNodes.emplace_back(Node{id, label, std::move(attributes)});
    mIdToIndex[id] = mNodes.size() - 1;
}

unsigned int Graph::getNumNodes() const {
    return mNodes.size();
}

void Graph::addEdge(Edge &e) {
    mEdges[e.source].push_back(e);
}

void Graph::addEdge(const std::string &source, const std::string &target, const std::string &label, double weight, double length) {
    mEdges[source].emplace_back(Edge{source, target, label, weight, length});
}

unsigned int Graph::getNumEdges() const {
    unsigned int total = 0;

    for (const auto &e : mEdges) {
        total += e.second.size();
    }

    return total;
}

void Graph::addAttribute(const std::string &s, std::unique_ptr<Attribute> value) {
    mAttributes.add(s, std::move(value));
}

bool Graph::isDirected() const {
    auto it = mAttributes.find("directed");

    if (it == mAttributes.end()) {
        return true;
    }

    return dynamic_cast<AttributeNumber *>(it->second.get())->getValue() == 1.0;
}
