#include "gml.h"
#include "attribute.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
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

static AttributeList parseAttributeList(size_t &i, const std::vector<Token> &tokens) {
    AttributeList attrList;
    i++;

    while (tokens[i].type != TokenType::CLOSE_BRACKET) {
        if (tokens[i].type == TokenType::ATTRIBUTE_NAME) {
            std::string attributeName = tokens[i].value;
            i++;

            if (tokens[i].type == TokenType::OPEN_BRACKET) {
                attrList.add(attributeName, std::make_shared<AttributeList>(parseAttributeList(i, tokens)));
            } else if (tokens[i].type == TokenType::ATTRIBUTE_STRING) {
                attrList.add(attributeName, std::make_shared<AttributeString>(AttributeString(tokens[i].value)));
            } else if (tokens[i].type == TokenType::ATTRIBUTE_NUMBER) {
                attrList.add(attributeName, std::make_shared<AttributeNumber>(AttributeNumber(std::stod(tokens[i].value))));
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
                        n.attributes[attributeName] = std::make_shared<AttributeList>(parseAttributeList(i, tokens));
                    } else if (tokens[i].type == TokenType::ATTRIBUTE_STRING) {
                        n.attributes[attributeName] = std::make_shared<AttributeString>(AttributeString(tokens[i].value));
                    } else if (tokens[i].type == TokenType::ATTRIBUTE_NUMBER) {
                        n.attributes[attributeName] = std::make_shared<AttributeNumber>(AttributeNumber(std::stod(tokens[i].value)));
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
                g.attributes[attributeName] = std::make_shared<AttributeString>(AttributeString(tokens[i].value));
            } else if (tokens[i].type == TokenType::ATTRIBUTE_NUMBER) {
                g.attributes[attributeName] = std::make_shared<AttributeNumber>(AttributeNumber(std::stod(tokens[i].value)));
            }
        }
    }

    return true;
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

    return;

    for (const auto &t : tokens) {
        std::string type;

        if (t.type == TokenType::OPEN_BRACKET) {
            type = "OPEN_BRACKET";
        } else if (t.type == TokenType::CLOSE_BRACKET) {
            type = "CLOSE_BRACKET";
        } else if (t.type == TokenType::GRAPH) {
            type = "GRAPH";
        } else if (t.type == TokenType::NODE) {
            type = "NODE";
        } else if (t.type == TokenType::EDGE) {
            type = "EDGE";
        } else if (t.type == TokenType::ATTRIBUTE_NAME) {
            type = "ATTRIBUTE_NAME";
        } else if (t.type == TokenType::ATTRIBUTE_STRING) {
            type = "ATTRIBUTE_STRING";
        } else if (t.type == TokenType::ATTRIBUTE_NUMBER) {
            type = "ATTRIBUTE_NUMBER";
        }

        std::cout << type << ", '" << t.value << "'" << std::endl;
    }
}

void GMLGraph::print() const {
    auto it = attributes.find("label");
    if (it != attributes.end()) {
        if (it->second->getType() == AttributeType::STRING) {
            auto label = *dynamic_cast<AttributeString *>(it->second.get());
            std::cout << "Label: " << label.value << std::endl;
        }
    }

    std::cout << "Nodes:" << std::endl;
    for (const auto &n : nodes) {
        std::cout << n.id << ", " << n.label << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Edges:" << std::endl;
    for (const auto &e : edges) {
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
