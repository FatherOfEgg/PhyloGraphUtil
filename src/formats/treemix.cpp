#include "treemix.h"

#include "eNewick.h"
#include <algorithm>
#include <cstddef>
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
    ORIGIN = 4,
    DESTINATION
};

struct Token {
    Token(TokenType t, uint64_t v)
    : type(t), value(v) {}

    TokenType type;
    uint64_t value;
};

static std::vector<uint64_t> getSubtreeId(const Graph &g, const std::string &word) {
    std::vector<std::string> leaves;

    size_t start = 0;
    size_t end = word.find(',');

    if (end != std::string::npos) {
        leaves.push_back(word.substr(start, end - start));
        start = end + 1;
    }

    leaves.push_back(word.substr(start));

    std::vector<uint64_t> res;
    res.reserve(leaves.size());

    for (const std::string &l : leaves) {
        start = 0;
        size_t colon = l.find(':');

        if (colon != std::string::npos) {
            if (l[0] == '(') {
                start = 1;
            }

            std::string leafName = l.substr(start, colon - start);
            for (const auto &p : g.leafName) {
                if (leafName == p.second) {
                    res.push_back(p.first);
                    break;
                }
            }
        }
    }

    return res;
}

static uint64_t extractSubtree(const Graph &g, const std::string &word) {
    std::vector<uint64_t> subtreeId = getSubtreeId(g, word);

    // Leaf
    if (subtreeId.size() == 1) {
        return subtreeId[0];
    }

    // Subtree of leaves: (a, b)
    for (uint64_t i = 0; i < g.adjList.size(); i++) {
        std::unordered_set<uint64_t> children(g.adjList[i].begin(), g.adjList[i].end());

        bool found = true;

        for (const uint64_t &id : subtreeId) {
            if (children.find(id) == children.end()) {
                found = false;
                break;
            }
        }

        if (found) {
            return i;
        }
    }

    std::cerr << "Couldn't find subtree for: (";
    std::cerr << g.leafName.at(subtreeId[0]);
    for (size_t i = 1; i < subtreeId.size(); i++) {
        std::cerr << ", " << g.leafName.at(subtreeId[i]);
    }
    std::cerr << ")" << std::endl;

    std::exit(EXIT_FAILURE);
}

static std::vector<Token> tokenize(std::ifstream &f, const Graph &g) {
    std::vector<Token> tokens;

    std::string line;

    std::getline(f, line);
    while (std::getline(f, line)) {
        std::istringstream ss(line);
        std::string word;
        std::vector<std::string> words;
        words.reserve(6);

        while (ss >> word) {
            words.push_back(word);
        }

        tokens.emplace_back(
            TokenType::ORIGIN,
            extractSubtree(g, words[static_cast<size_t>(TokenType::ORIGIN)])
        );

        tokens.emplace_back(
            TokenType::DESTINATION,
            extractSubtree(g, words[static_cast<size_t>(TokenType::DESTINATION)])
        );
    }

    return tokens;
}

static bool parse(Graph &g, const std::vector<Token> &tokens) {
    if (tokens.empty()) {
        return false;
    }

    uint64_t origin = 0;

    for (const Token &t : tokens) {
        uint64_t subtree = t.value;

        uint64_t newNode = g.adjList.size();
        g.addNode();

        for (size_t i = 0; i < g.adjList.size(); i++) {
            std::vector<uint64_t> &children = g.adjList[i];
            auto it = std::find(children.begin(), children.end(), subtree);

            if (it != children.end()) {
                if (t.type == TokenType::ORIGIN) {
                    *it = newNode;
                    origin = newNode;
                    g.addEdge(newNode, subtree);
                } else {
                    *it = newNode;
                    g.addEdge(newNode, subtree);
                    g.addEdge(origin, newNode);

                    g.reticulations[newNode].push_back(origin);
                    g.reticulations[newNode].push_back(i);
                }

                break;
            }
        }
    }

    return true;
}

bool openTreemix(Graph &g, const std::string &file) {
    if (!openENWK(g, file)) {
        return false;
    }

    std::ifstream f(file);

    if (!f.is_open()) {
        return false;
    }

    std::vector<Token> tokens = tokenize(f, g);
    f.close();

    return parse(g, tokens);
}

void saveTreemix(const Graph &g, const std::string &filename) {
    std::cout << "Saving as treemix has not been implemented yet." << std::endl;
}
