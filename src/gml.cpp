#include "gml.h"
#include "attribute.h"
#include "graph.h"

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

    while (true) {
        int v = f.peek();

        if (v == EOF) {
            break;
        }

        c = static_cast<char>(v);
        if (isalpha(c)) {
            std::string word;

            while (f.get(c) && isalpha(c)) {
                word += c;
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
            f.get(c);
        } else if (c == ']') {
            tokens.push_back({TokenType::CLOSE_BRACKET, "]"});
            f.get(c);
        } else if (c == '"') {
            std::string s;

            f.get(c);
            s += c;
            while (f.get(c) && c != '"') {
                s += c;
            }
            s += '"';

            tokens.push_back({TokenType::ATTRIBUTE_STRING, s});
        } else if (isdigit(c) || c == '-' || c == '.') {
            std::string n;

            while (f.get(c) && (isdigit(c) || c == '-' || c == '.')) {
                n += c;
            }

            tokens.push_back({TokenType::ATTRIBUTE_NUMBER, n});
        } else {
            f.get(c);
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
    std::cout << "Save as GML" << std::endl;
}

void GMLGraph::print() const {
    std::cout << "Graph attributes:" << std::endl;
    for (auto &pair : mAttributes) {
        std::string attributeName = pair.first;
        std::string value;

        if (pair.second->getType() == AttributeType::STRING) {
            value = dynamic_cast<AttributeString *>(pair.second.get())->getValue();
        } else if (pair.second->getType() == AttributeType::NUMBER) {
            value = std::to_string(dynamic_cast<AttributeNumber *>(pair.second.get())->getValue());
        }

        std::cout << attributeName << ": " << value << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Nodes:" << std::endl;
    for (const auto &n : mNodes) {
        std::cout << n.id << ", " << n.label << std::endl;
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
