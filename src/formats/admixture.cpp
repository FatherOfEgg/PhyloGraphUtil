#include "admixture.h"
#include "../graph.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

enum class TokenType {
    ID,
    SOURCE,
    TARGET,
    TYPE,
    NUM
};

struct Token {
public:
    Token(TokenType type, const std::string &value) : type(type), value(value) {}

public:
    TokenType type;
    std::string value;
};

static bool isNumber(const std::string &s) {
    return !s.empty() && s.find_first_not_of("0123456789") == std::string::npos;
}

static bool containsNumber(const std::string &s) {
    return !s.empty() && s.find_first_of("0123456789-") != std::string::npos;
}

static std::vector<Token> tokenize(std::ifstream &f) {
    std::vector<Token> tokens;

    std::string line;

    while (std::getline(f, line)) {
        std::istringstream ss(line);
        std::string word;
        std::vector<std::string> words;

        while (ss >> word) {
            words.push_back(word);
        }

        for (size_t i = 0; i < words.size(); i++) {
            if (i >= static_cast<size_t>(TokenType::NUM)) {
                continue;
            }

            TokenType type = static_cast<TokenType>(i);

            if (type == TokenType::ID) {
                if (!isNumber(words[i])) {
                    return std::vector<Token>();
                }

            } else if (type == TokenType::TYPE) {
                if (containsNumber(words[i])) {
                    return std::vector<Token>();
                }
            }

            if (type == TokenType::SOURCE || type == TokenType::TARGET) {
                tokens.emplace_back(type, words[i]);
            } else if (type == TokenType::ID || type == TokenType::TYPE) {
                tokens.emplace_back(type, words[i]);
            }
        }
    }

    return tokens;
}

static bool parse(Graph &g, const std::vector<Token> &tokens) {
    if (tokens.empty()) {
        return false;
    }

    std::unordered_map<std::string, uint64_t> idToIndex;
    std::unordered_map<uint64_t, std::string> indexToId;

    TokenType nextExpectedType = TokenType::ID;

    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].type != nextExpectedType) {
            return false;
        }

        switch (tokens[i].type) {
            case TokenType::ID:
                nextExpectedType = TokenType::SOURCE;
                break;
            case TokenType::SOURCE:
                {
                    auto it = idToIndex.find(tokens[i].value);

                    if (it == idToIndex.end()) {
                        indexToId[idToIndex.size()] = tokens[i].value;
                        idToIndex[tokens[i].value] = idToIndex.size();

                        g.addNode();
                    }

                }

                nextExpectedType = TokenType::TARGET;
                break;
            case TokenType::TARGET:
                {
                    auto it = idToIndex.find(tokens[i].value);

                    uint64_t source = idToIndex.at(tokens[i - 1].value);
                    uint64_t target;

                    if (it == idToIndex.end()) {
                        target = idToIndex.size();
                        indexToId[target] = tokens[i].value;
                        idToIndex[tokens[i].value] = target;

                        g.addNode();
                    } else {
                        target = it->second;
                    }

                    g.addEdge(source, target);
                }

                nextExpectedType = TokenType::TYPE;
                break;
            case TokenType::TYPE:
                {
                    if (tokens[i].value == "admix") {
                        uint64_t reticulation = idToIndex.at(tokens[i - 1].value);
                        uint64_t parent = idToIndex.at(tokens[i - 2].value);

                        g.reticulations[reticulation].push_back(parent);
                    }
                }

                nextExpectedType = TokenType::ID;
                break;
            default:
                break;
        }
    }

    for (size_t i = 0; i < g.adjList.size(); i++) {
        if (g.adjList[i].empty()) {
            g.leaves.push_back(i);
            g.leafName[i] = indexToId[i];
        }
    }

    return true;
}

bool openADMIX(Graph &g, const std::string &file) {
    std::ifstream f(file);

    if (!f.is_open()) {
        return false;
    }

    std::vector<Token> tokens = tokenize(f);
    f.close();

    return parse(g, tokens);
}

void saveADMIX(const Graph &g, const std::string &filename) {
    std::ofstream f(filename);

    if (!f) {
        std::cerr << "Failed to save to '" << filename << "'."<< std::endl;
        std::exit(EXIT_FAILURE);
    }

    uint64_t edgeCount = 1;

    f << std::left;

    for (size_t s = 0; s < g.adjList.size(); s++) {
        for (size_t t = 0; t < g.adjList[s].size(); t++) {
            f << std::setw(4) << std::to_string(edgeCount);
            edgeCount++;

            f << std::setw(7) << "n" + std::to_string(s);

            auto leafIt = g.leafName.find(g.adjList[s][t]);

            f << std::setw(7);
            if (leafIt != g.leafName.end()) {
                f << leafIt->second;
            } else {
                f << "n" + std::to_string(g.adjList[s][t]);
            }

            auto reticIt = g.reticulations.find(g.adjList[s][t]);

            f << std::setw(7);
            if (reticIt != g.reticulations.end()) {
                f << "admix";
            } else {
                f << "edge";
            }

            f << std::setw(8);
            f << "0";

            f << std::setw(8);
            if (reticIt != g.reticulations.end()) {
                f << "0.5";
            } else {
                f << "NA";
            }

            if (reticIt != g.reticulations.end()) {
                f << "0.5";
            } else {
                f << "NA";
            }

            f << std::endl;
        }
    }

    f.close();
}
