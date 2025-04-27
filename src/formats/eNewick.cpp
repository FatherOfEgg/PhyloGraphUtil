#include "eNewick.h"
#include "../graph.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

enum class TokenType {
    OPEN_PARENTHESIS,
    CLOSE_PARENTHESIS,
    SEMI_COLON,
    LENGTH,
    INTERNAL_NAME,
    LEAF_NAME,
    HYBRID_ID,
};

struct Token {
    TokenType type;
    std::string value;
};

static std::vector<Token> tokenize(std::ifstream &f) {
    std::vector<Token> tokens;
    char c;

    while (f.get(c)) {
        if (c == ':') {
            f.get(c);
            std::string length = {c};

            int n = f.peek();
            while (n != EOF && (isdigit(n) || n == '.')) {
                f.get(c);
                length += c;
                n = f.peek();
            }

            tokens.push_back({TokenType::LENGTH, length});
        } else if (isalpha(c) || isdigit(c)) {
            std::string name = {c};

            int n = f.peek();
            while (n != EOF && (isalpha(n) || isdigit(n))) {
                f.get(c);
                name += c;
                n = f.peek();
            }

            TokenType t;
            if (!tokens.empty()
            &&  tokens.back().type == TokenType::CLOSE_PARENTHESIS) {
                t = TokenType::INTERNAL_NAME;
            } else {
                t = TokenType::LEAF_NAME;
            }

            tokens.push_back({t, name});
        } else if (c == '#') {
            std::string hybrid_id;

            int n = f.peek();

            if (n == 'H') {
                f.get(c);
            }

            n = f.peek();
            while (n != EOF && isdigit(n)) {
                f.get(c);
                hybrid_id += c;
                n = f.peek();
            }

            if (tokens.back().type == TokenType::OPEN_PARENTHESIS
            ||  tokens.back().type == TokenType::HYBRID_ID) {
                tokens.push_back({TokenType::LEAF_NAME, ""});
            }

            tokens.push_back({TokenType::HYBRID_ID, hybrid_id});
        } else if (c == '(') {
            tokens.push_back({TokenType::OPEN_PARENTHESIS, "("});
        } else if (c == ')') {
            tokens.push_back({TokenType::CLOSE_PARENTHESIS, ")"});

            int n = f.peek();
            if (n != EOF && (!isalpha(n) && !isdigit(n))) {
                tokens.push_back({TokenType::INTERNAL_NAME, ""});
            }
        } else if (c == ';') {
            tokens.push_back({TokenType::SEMI_COLON, ";"});

            // For now, we break once we hit a ;
            // Will maybe add support if the extended newick file
            // has multiple lines in it.
            break;
        }
    }

    return tokens;
}

static bool parse(Graph &g, const std::vector<Token> &tokens) {
    if (tokens.front().type != TokenType::OPEN_PARENTHESIS
    ||  tokens.back().type != TokenType::SEMI_COLON) {
        return false;
    }

    uint64_t curIndex = 0;
    std::stack<std::vector<uint64_t>> children;

    bool isLeafHybrid = false;
    std::unordered_map<std::string, uint64_t> hybrids;

    for (size_t i = 0; i < tokens.size(); i++) {
        // I think it's just easier to handle the case where a
        // hybrid is a leaf node "separately"
        // (In HYBRID_ID instead of LEAF_NAME)
        if (tokens[i].type == TokenType::HYBRID_ID) {
            if (isLeafHybrid) {
                children.pop();
            }

            auto it = hybrids.find(tokens[i].value);
            if (it != hybrids.end()) {
                children.top().push_back(it->second);

                if (isLeafHybrid) {
                    g.leaves.push_back(it->second);
                    g.leafName[it->second] = tokens[i - 3].value;
                }
            } else {
                g.addNode();
                children.top().push_back(curIndex);

                if (isLeafHybrid) {
                    g.leaves.push_back(curIndex);
                    g.leafName[curIndex] = tokens[i - 3].value;
                }

                g.reticulations[curIndex];
                hybrids[tokens[i].value] = curIndex;

                curIndex++;
            }

            isLeafHybrid = false;
        } else if (tokens[i].type == TokenType::LEAF_NAME) {
            if (tokens[i - 1].type == TokenType::OPEN_PARENTHESIS
            &&  tokens[i + 1].type == TokenType::CLOSE_PARENTHESIS
            &&  tokens[i + 3].type == TokenType::HYBRID_ID) {
                isLeafHybrid = true;
                i += 2;
                continue;
            }

            if (tokens[i + 1].type == TokenType::HYBRID_ID) {
                continue;
            }

            g.addNode();
            children.top().push_back(curIndex);

            g.leaves.push_back(curIndex);
            g.leafName[curIndex] = tokens[i].value;

            curIndex++;
        } else if (tokens[i].type == TokenType::INTERNAL_NAME) {
            bool isHybrid = tokens[i + 1].type == TokenType::HYBRID_ID;
            auto hybridIt = hybrids.find(tokens[i + 1].value);

            if (isHybrid && hybridIt == hybrids.end()) {
                g.reticulations[curIndex];
                hybrids[tokens[i + 1].value] = curIndex;
            }

            bool addedNode = false;

            if (!children.top().empty()) {
                uint64_t nodeIndex = curIndex;

                if (isHybrid) {
                    nodeIndex = hybrids.at(tokens[i + 1].value);
                }

                if (nodeIndex >= curIndex) {
                    g.addNode();
                    addedNode = true;
                }

                /* std::cout << nodeIndex << std::endl;
                for (const auto &e : children.top()) {
                    std::cout << e << ", " << std::endl;
                } */

                g.adjList[nodeIndex] = std::move(children.top());
                children.pop();

                for (const uint64_t &n : g.adjList[nodeIndex]) {
                    auto it = g.reticulations.find(n);

                    if (it != g.reticulations.end()) {
                        g.reticulations[n].push_back(curIndex);
                    }
                }
            }

            if (tokens[i + 1].type == TokenType::SEMI_COLON) {
                continue;
            }

            if (isHybrid) {
                children.top().push_back(hybrids.at(tokens[i + 1].value));

                i++;
            } else {
                children.top().push_back(curIndex);
            }

            if (addedNode) {
                curIndex++;
            }
        } else if (tokens[i].type == TokenType::OPEN_PARENTHESIS) {
            children.emplace();
        }
    }

    return true;
}

bool openENWK(Graph &g, const std::string &file) {
    std::ifstream f(file);

    if (!f.is_open()) {
        return false;
    }

    std::vector<Token> tokens = tokenize(f);
    f.close();

    return parse(g, tokens);
}

static void nextHybridName(std::string &name) {
    for (size_t i = name.length() - 1; i >= 0; i--) {
        if (name[i] == 'z') {
            name[i] = 'a';
        } else {
            name[i]++;
            return;
        }
    }

    name = "a" + name;
}

static std::unordered_map<uint64_t, std::string> assignHybridStr(const Graph &g) {
    std::unordered_map<uint64_t, std::string> res;

    uint64_t hybridId = 1;
    std::string hybridName = "a";

    for (const auto &r : g.reticulations) {
        res[r.first] = hybridName + "#H" + std::to_string(hybridId);
        hybridId++;
        nextHybridName(hybridName);
    }

    return res;
}

void saveENWK(const Graph &g, const std::string &filename) {
    std::ofstream f(filename);

    if (!f) {
        std::cerr << "Failed to save to '" << filename << "'."<< std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::unordered_set<uint64_t> hybridFirstOccurrence;
    std::unordered_map<uint64_t, std::string> hybridStr = assignHybridStr(g);

    std::function<void (uint64_t)> dfs = [&](uint64_t node) {
        auto leafIt = g.leafName.find(node);
        bool isHybrid = g.reticulations.find(node) != g.reticulations.end();

        if (leafIt != g.leafName.end()) {
            if (isHybrid) {
                bool isFirstOccurrence = hybridFirstOccurrence.find(node) == hybridFirstOccurrence.end();

                if (isFirstOccurrence) {
                    f << "(" << leafIt->second << ")";

                    hybridFirstOccurrence.insert(node);
                }

                f << hybridStr.at(node);
            } else {
                f << leafIt->second;
            }

            return;
        }

        size_t numChildren = g.adjList[node].size();

        if (numChildren != 0) {
            f << "(";

            for (size_t i = 0; i < numChildren; i++) {
                dfs(g.adjList[node][i]);

                if (i != numChildren - 1) {
                    f << ", ";
                }
            }

            f << ")";
        }

        // if (includeInternalNames) {
        //     res += nodeID;
        // }

        if (isHybrid) {
            f << hybridStr.at(node);
        }
    };


    dfs(g.root);
    f << ";";

    f.close();
}
