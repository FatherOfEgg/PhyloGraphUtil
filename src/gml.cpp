#include "gml.h"
#include "attribute.h"
#include "graph.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

enum class TokenType {
    OPEN_BRACKET,
    CLOSE_BRACKET,
    GRAPH,
    NODE,
    EDGE,
    ATTRIBUTE_NAME,
    ATTRIBUTE_STRING,
    ATTRIBUTE_NUMBER,
};

struct Token {
    TokenType type;
    std::string value;
};

static std::vector<Token> tokenize(std::ifstream &f) {
    std::vector<Token> tokens;
    char c;

    while (f.get(c)) {
        if (isalpha(c)) {
            std::string word = {c};

            int n = f.peek();
            while (n != EOF && isalpha(n)) {
                f.get(c);
                word += c;
                n = f.peek();
            }

            TokenType t;
            if (word == "graph") {
                t = TokenType::GRAPH;
            } else if (word == "node") {
                t = TokenType::NODE;
            } else if (word == "edge") {
                t = TokenType::EDGE;
            } else if (tokens.back().type == TokenType::ATTRIBUTE_NAME) {
                t = TokenType::ATTRIBUTE_STRING;
            } else {
                t = TokenType::ATTRIBUTE_NAME;
            }

            tokens.push_back({t, word});
        } else if (c == '[') {
            tokens.push_back({TokenType::OPEN_BRACKET, "["});
        } else if (c == ']') {
            tokens.push_back({TokenType::CLOSE_BRACKET, "]"});
        } else if (c == '"') {
            std::string s;

            int n = f.peek();
            while (n != EOF && n != '"') {
                f.get(c);
                s += c;
                n = f.peek();
            }
            f.get(c);

            tokens.push_back({TokenType::ATTRIBUTE_STRING, s});
        } else if (isdigit(c) || c == '-' || c == '.') {
            std::string num = {c};

            int n = f.peek();
            while (n != EOF && (isdigit(n) || n == '-' || n == '.')) {
                f.get(c);
                num += c;
                n = f.peek();
            }

            tokens.push_back({TokenType::ATTRIBUTE_NUMBER, num});
        }
    }

    return tokens;
}

static AttributeMap parseAttributeList(size_t &i, const std::vector<Token> &tokens) {
    AttributeMap attrList;
    i++;

    while (tokens[i].type != TokenType::CLOSE_BRACKET) {
        if (tokens[i].type == TokenType::ATTRIBUTE_NAME) {
            std::string attributeName = tokens[i].value;
            i++;

            if (tokens[i].type == TokenType::OPEN_BRACKET) {
                attrList.add(attributeName, std::make_unique<AttributeMap>(parseAttributeList(i, tokens)));
            } else if (tokens[i].type == TokenType::ATTRIBUTE_STRING) {
                attrList.add(attributeName, std::make_unique<AttributeString>(tokens[i].value));
            } else if (tokens[i].type == TokenType::ATTRIBUTE_NUMBER) {
                attrList.add(attributeName, std::make_unique<AttributeNumber>(std::stod(tokens[i].value)));
            }
        }

        i++;
    }

    return attrList;
}

static bool parse(GMLGraph &g, const std::vector<Token> &tokens) {
    if (tokens.front().type != TokenType::GRAPH
    ||  tokens[1].type != TokenType::OPEN_BRACKET) {
        return false;
    }

    for (size_t i = 2; i < tokens.size(); i++) {
        if (tokens[i].type == TokenType::NODE) {
            i++;

            if (tokens[i].type != TokenType::OPEN_BRACKET) {
                return false;
            }

            i++;
            Node n;
            while (tokens[i].type != TokenType::CLOSE_BRACKET) {
                if (tokens[i].type == TokenType::ATTRIBUTE_NAME) {
                    std::string attributeName = tokens[i].value;
                    i++;

                    if (attributeName == "id") {
                        n.id = tokens[i].value;
                    } else if (attributeName == "label") {
                        n.label = tokens[i].value;
                    } else if (tokens[i].type == TokenType::OPEN_BRACKET) {
                        n.attributes.add(attributeName, std::make_unique<AttributeMap>(parseAttributeList(i, tokens)));
                    } else if (tokens[i].type == TokenType::ATTRIBUTE_STRING) {
                        n.attributes.add(attributeName, std::make_unique<AttributeString>(tokens[i].value));
                    } else if (tokens[i].type == TokenType::ATTRIBUTE_NUMBER) {
                        n.attributes.add(attributeName, std::make_unique<AttributeNumber>(std::stod(tokens[i].value)));
                    }
                }

                i++;
            }

            g.addNode(n);
        } else if (tokens[i].type == TokenType::EDGE) {
            i++;

            if (tokens[i].type != TokenType::OPEN_BRACKET) {
                return false;
            }

            i++;
            Edge e = {.weight = 1.0, .length = 1.0};
            while (tokens[i].type != TokenType::CLOSE_BRACKET) {
                if (tokens[i].type == TokenType::ATTRIBUTE_NAME) {
                    std::string attributeName = tokens[i].value;
                    i++;

                    if (attributeName == "source") {
                        e.source = tokens[i].value;
                    } else if (attributeName == "target") {
                        e.target = tokens[i].value;
                    } else if (attributeName == "label") {
                        e.label = tokens[i].value;
                    } else if (attributeName == "weight") {
                        e.weight = std::stod(tokens[i].value);
                    } else if (attributeName == "length") {
                        e.length = std::stod(tokens[i].value);
                    }
                }

                i++;
            }

            g.addEdge(e);
        } else if (tokens[i].type == TokenType::ATTRIBUTE_NAME) { // Graph attributes
            std::string attributeName = tokens[i].value;
            i++;

            if (tokens[i].type == TokenType::ATTRIBUTE_STRING) {
                g.addAttribute(attributeName, std::make_unique<AttributeString>(tokens[i].value));
            } else if (tokens[i].type == TokenType::ATTRIBUTE_NUMBER) {
                g.addAttribute(attributeName, std::make_unique<AttributeNumber>(std::stod(tokens[i].value)));
            }
        }
    }

    return true;
}

GMLGraph::GMLGraph() {}

GMLGraph::GMLGraph(const Graph &other)
: Graph(other) {}

GMLGraph &GMLGraph::operator=(const Graph &other) {
    if (this != &other) {
        Graph::operator=(other);
    }

    return *this;
}

void GMLGraph::addEdge(Edge &e) {
    Graph::addEdge(e);

    if (!isDirected()) {
        auto edgeIt = std::find_if(
            mEdges[e.target].begin(),
            mEdges[e.target].end(),
            [e](const Edge &edge) {
                return edge.target == e.source;
            }
        );

        if (edgeIt == mEdges[e.target].end()) {
            mEdges[e.target].emplace_back(Edge{e.target, e.source, e.label, e.weight, e.length});
        }
    }
}

void GMLGraph::addEdge(const std::string &source, const std::string &target, const std::string &label, double weight, double length) {
    Graph::addEdge(source, target, label, weight, length);

    if (!isDirected()) {
        auto edgeIt = std::find_if(
            mEdges[target].begin(),
            mEdges[target].end(),
            [source](const Edge &edge) {
                return edge.target == source;
        });

        if (edgeIt == mEdges[target].end()) {
            mEdges[target].emplace_back(Edge{target, source, label, weight, length});
        }
    }
}

void GMLGraph::open(const std::string &file) {
    std::ifstream f(file);

    if (!f.is_open()) {
        std::cerr << "Couldn't open `" << file << "`." << std::endl;
        std::exit(1);
    }

    std::vector<Token> tokens = tokenize(f);
    f.close();

    if (!parse(*this, tokens)) {
        std::cerr << "'" << file << "' is not a valid GML file." << std::endl;
        std::exit(1);
    }
}

void GMLGraph::save(const std::string &filename) const {
    std::ofstream f(filename);

    if (!f) {
        std::cerr << "Failed to save to '" << filename << "'."<< std::endl;
        std::exit(1);
    }

    std::string indent = "    ";

    f << "graph [" << std::endl;

    for (const auto &pair : mAttributes) {
        std::string attribute = pair.first + " ";

        if (pair.second->getType() == AttributeType::STRING) {
            attribute += "\"";
            attribute += dynamic_cast<AttributeString *>(pair.second.get())->getValue();
            attribute += "\"";
        } else if (pair.second->getType() == AttributeType::NUMBER) {
            attribute = std::to_string(dynamic_cast<AttributeNumber *>(pair.second.get())->getValue());
        }

        f << indent << attribute << std::endl;
    }

    for (const auto &n : mNodes) {
        f << indent << "node [" << std::endl;
        indent += "    ";

        f << indent << "id " << n.id << std::endl;
        f << indent << "label \"" << n.label << "\"" << std::endl;

        indent = indent.substr(0, indent.length() - 4);
        f << indent << "]" << std::endl;
    }

    for (const auto &e : mEdges) {
        std::string source = e.first;

        for (const auto &adj : e.second) {
            f << indent << "edge [" << std::endl;
            indent += "    ";

            f << indent << "source " << source << std::endl;
            f << indent << "target " << adj.target << std::endl;
            f << indent << "label \"" << adj.label << "\"" << std::endl;

            indent = indent.substr(0, indent.length() - 4);
            f << indent << "]" << std::endl;
        }
    }

    f << "]";

    f.close();
}

void GMLGraph::print() const {
    std::cout << "Graph attributes:" << std::endl;
    for (auto &pair : mAttributes) {
        std::string attributeName = pair.first;
        std::string value;

        if (pair.second->getType() == AttributeType::STRING) {
            value += "\"";
            value += dynamic_cast<AttributeString *>(pair.second.get())->getValue();
            value += "\"";
        } else if (pair.second->getType() == AttributeType::NUMBER) {
            value = std::to_string(dynamic_cast<AttributeNumber *>(pair.second.get())->getValue());
        }

        std::cout << attributeName << ": " << value << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Nodes:" << std::endl;
    for (const auto &n : mNodes) {
        std::cout << n.id << ", \"" << n.label << "\"" << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Edges:" << std::endl;
    for (const auto &e : mEdges) {
        std::cout << e.first << ": ";

        for (const auto &x : e.second) {
            std::cout << x.target << ", ";
        }

        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "NumNodes: " << getNumNodes() << std::endl;
    std::cout << "NumEdges: " << getNumEdges() << std::endl;
}
