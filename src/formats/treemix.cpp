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
#include <vector>

enum class TokenType {
    ORIGIN = 4,
    DESTINATION
};

struct Token {
    Token(TokenType t, std::vector<std::string> v)
    : type(t), value(std::move(v)) {}

    TokenType type;
    std::vector<std::string> value;
};

static std::vector<std::string> extractSubtree(const std::string &word) {
    std::vector<std::string> res;
    std::vector<std::string> leaves;

    size_t start = 0;
    size_t end = word.find(',');

    if (end != std::string::npos) {
        leaves.push_back(word.substr(start, end - start));
        start = end + 1;
    }

    leaves.push_back(word.substr(start));

    for (const std::string &l : leaves) {
        start = 0;
        size_t colon = l.find(':');

        if (colon != std::string::npos) {
            if (l[0] == '(') {
                start = 1;
            }

            res.push_back(l.substr(start, colon - start));
        }
    }

    return res;
}

static std::vector<Token> tokenize(std::ifstream &f) {
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
            extractSubtree(words[static_cast<size_t>(TokenType::ORIGIN)])
        );

        tokens.emplace_back(
            TokenType::DESTINATION,
            extractSubtree(words[static_cast<size_t>(TokenType::DESTINATION)])
        );
    }

    return tokens;
}

uint64_t findSubtree() {

}

static bool parse(Graph &g, const std::vector<Token> &tokens) {
    if (tokens.empty()) {
        return false;
    }

    uint64_t origin;

    for (const Token &t : tokens) {
        uint64_t subtree = findSubtree();

        uint64_t newNode = g.adjList.size();
        g.addNode();

        for (auto &children : g.adjList) {
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

    std::vector<Token> tokens = tokenize(f);
    f.close();

    return parse(g, tokens);
}

void saveTreemix(const Graph &g, const std::string &filename) {
    std::cout << "Saving as treemix has not been implemented yet." << std::endl;
}
