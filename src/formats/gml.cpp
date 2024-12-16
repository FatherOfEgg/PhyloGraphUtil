#include "gml.h"
#include "../graph.h"

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum class TokenType {
    OPEN_BRACKET,
    CLOSE_BRACKET,
    GRAPH,
    DIRECTED,
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
            } else if (word == "directed") {
                t = TokenType::DIRECTED;
            } else if (word == "node") {
                t = TokenType::NODE;
            } else if (word == "edge") {
                t = TokenType::EDGE;
            } else if (!tokens.empty() && tokens.back().type == TokenType::ATTRIBUTE_NAME) {
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

            if (!tokens.empty()
            &&  tokens.back().type == TokenType::DIRECTED
            &&  std::stoi(num, nullptr) == 0) {
                std::cerr << "GML graph is not directed. Please make sure it is directed." << std::endl;
                std::exit(EXIT_FAILURE);
            }

            tokens.push_back({TokenType::ATTRIBUTE_NUMBER, num});
        }
    }

    return tokens;
}

static bool parse(Graph &g, const std::vector<Token> &tokens) {
    if (tokens.front().type != TokenType::GRAPH
    ||  tokens[1].type != TokenType::OPEN_BRACKET) {
        return false;
    }

    uint64_t curIndex = 0;
    std::unordered_map<std::string, uint64_t> idToIndex;
    std::unordered_map<uint64_t, std::string> indexToId;

    std::unordered_set<uint64_t> reticulations;
    std::unordered_map<uint64_t, std::vector<uint64_t>> nodeParents;

    for (size_t i = 2; i < tokens.size(); i++) {
        if (tokens[i].type == TokenType::NODE) {
            i++;

            if (tokens[i].type != TokenType::OPEN_BRACKET) {
                return false;
            }

            i++;
            while (tokens[i].type != TokenType::CLOSE_BRACKET) {
                if (tokens[i].type == TokenType::ATTRIBUTE_NAME) {
                    std::string attributeName = tokens[i].value;
                    i++;

                    if (attributeName == "id") {
                        idToIndex[tokens[i].value] = curIndex;
                        indexToId[curIndex] = tokens[i].value;
                        g.addNode();

                        curIndex++;
                    } else if (tokens[i].type == TokenType::OPEN_BRACKET) {
                        unsigned int bracketCount = 1;

                        while (bracketCount) {
                            i++;

                            if (tokens[i].type == TokenType::OPEN_BRACKET) {
                                bracketCount++;
                            } else if (tokens[i].type == TokenType::CLOSE_BRACKET) {
                                bracketCount--;
                            }
                        }
                    }
                }

                i++;
            }
        } else if (tokens[i].type == TokenType::EDGE) {
            i++;

            if (tokens[i].type != TokenType::OPEN_BRACKET) {
                return false;
            }

            i++;
            uint64_t source;
            uint64_t target;

            while (tokens[i].type != TokenType::CLOSE_BRACKET) {
                if (tokens[i].type == TokenType::ATTRIBUTE_NAME) {
                    std::string attributeName = tokens[i].value;
                    i++;

                    if (attributeName == "source") {
                        source = idToIndex[tokens[i].value];
                    } else if (attributeName == "target") {
                        target = idToIndex[tokens[i].value];
                    }
                }

                i++;
            }

            nodeParents[target].push_back(source);

            if (nodeParents[target].size() >= 2) {
                reticulations.insert(target);
            }

            g.addEdge(source, target);
        }
    }

    for (const uint64_t &r : reticulations) {
        g.reticulations[r] = nodeParents[r];
    }

    for (size_t i = 0; i < g.adjList.size(); i++) {
        if (g.adjList[i].empty()) {
            g.leaves.push_back(i);
            g.leafName[i] = indexToId[i];
        }
    }

    return true;
}

bool openGML(Graph &g, const std::string &file) {
    std::ifstream f(file);

    if (!f.is_open()) {
        return false;
    }

    std::vector<Token> tokens = tokenize(f);
    f.close();

    return parse(g, tokens);
}

void saveGML(const Graph &g, const std::string &filename) {
    std::ofstream f(filename);

    if (!f) {
        std::cerr << "Failed to save to '" << filename << "'."<< std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::ostringstream edges;

    std::string indent = "    ";

    f << "graph [" << std::endl;
    f << indent << "directed 1" << std::endl;
    
    for (size_t i = 0; i < g.adjList.size(); i++) {
        f << indent << "node [" << std::endl;
        indent += "    ";

        f << indent << "id " << i << std::endl;

        indent = indent.substr(0, indent.length() - 4);
        f << indent << "]" << std::endl;

        for (const auto &t : g.adjList[i]) {
            edges << indent << "edge [" << std::endl;
            indent += "    ";

            edges << indent << "source " << i << std::endl;
            edges << indent << "target " << t<< std::endl;

            indent = indent.substr(0, indent.length() - 4);
            edges << indent << "]" << std::endl;
        }
    }

    f << edges.str();
    f << "]";

    f.close();
}
