#include "eNewick.h"
#include "../graph.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
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
            if (tokens.back().type == TokenType::CLOSE_PARENTHESIS) {
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
    std::stack<uint64_t> edgeTargets;
    std::stack<uint64_t> numChildren;

    bool isLeafHybrid = false;
    std::unordered_map<std::string, uint64_t> hybrids;

    for (size_t i = 0; i < tokens.size(); i++) {
        // I think it's just easier to handle the case where a
        // hybrid is a leaf node "separately"
        // (In HYBRID_ID instead of LEAF_NAME)
        if (tokens[i].type == TokenType::HYBRID_ID) {
            if (isLeafHybrid) {
                numChildren.pop();
            }

            auto it = hybrids.find(tokens[i].value);
            if (it != hybrids.end()) {
                edgeTargets.push(it->second);

                if (isLeafHybrid) {
                    g.leaves.push_back(it->second);
                    g.leafName[it->second] = tokens[i - 3].value;
                }
            } else {
                g.addNode();
                edgeTargets.push(curIndex);

                if (isLeafHybrid) {
                    g.leaves.push_back(curIndex);
                    g.leafName[curIndex] = tokens[i - 3].value;
                }

                g.reticulations[curIndex];
                hybrids[tokens[i].value] = curIndex;

                curIndex++;
            }

            numChildren.top()++;
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
            edgeTargets.push(curIndex);

            g.leaves.push_back(curIndex);
            g.leafName[curIndex] = tokens[i].value;

            curIndex++;

            numChildren.top()++;
        } else if (tokens[i].type == TokenType::INTERNAL_NAME) {
            bool isHybrid = tokens[i + 1].type == TokenType::HYBRID_ID;
            auto hybridIt = hybrids.find(tokens[i + 1].value);

            if (isHybrid && hybridIt == hybrids.end()) {
                g.addNode();

                g.reticulations[curIndex];
                hybrids[tokens[i + 1].value] = curIndex;

                curIndex++;
            }

            for (unsigned int c = 0; c < numChildren.top(); c++) {
                uint64_t target = edgeTargets.top();

                if (isHybrid) {
                    g.addEdge(hybrids.at(tokens[i + 1].value), target);
                } else {
                    g.addEdge(curIndex, target);
                }

                auto it = g.reticulations.find(target);
                if (it != g.reticulations.end()) {
                    g.reticulations[target].push_back(curIndex);
                }

                edgeTargets.pop();
            }

            if (tokens[i + 1].type == TokenType::SEMI_COLON) {
                continue;
            }

            numChildren.pop();

            if (isHybrid) {
                edgeTargets.push(hybrids.at(tokens[i + 1].value));

                i++;
            } else {
                g.addNode();
                edgeTargets.push(curIndex);

                curIndex++;
            }

            if (!numChildren.empty()) {
                numChildren.top()++;
            }
        } else if (tokens[i].type == TokenType::OPEN_PARENTHESIS) {
            numChildren.push(0);
        }
    }

    return true;
}

// std::unordered_map<std::string, std::string> getHybrids(const Graph &g) {
//     std::unordered_map<std::string, std::string> hybrids;
//     std::unordered_map<std::string, unsigned int> incomingCount;
//
//     for (const auto &e : g.edges) {
//         for (const auto &x : e.second) {
//             incomingCount[x.target]++;
//         }
//     }
//
//     unsigned int hybridID = 1;
//     for (const auto &n : incomingCount) {
//         if (n.second >= 2) {
//             hybrids[n.first] = "#H" + std::to_string(hybridID);
//         }
//     }
//
//     return hybrids;
// }
//
// std::string dfs(
//     const Graph &g,
//     const std::string &nodeID,
//     const std::unordered_map<std::string,
//     std::string> &hybrids,
//     bool includeInternalNames
// ) {
//     auto it = g.edges.find(nodeID);
//     if (it == g.edges.end()
//     ||  it->second.empty()) {
//         auto n = std::find_if(
//             g.nodes.begin(),
//             g.nodes.end(),
//             [&nodeID](const Node &node) {
//                 return node.id == nodeID;
//             }
//         );
//         return n->id;
//     }
//
//     std::vector<Edge> neighbors = it->second;
//     std::vector<std::string> out;
//
//     for (const auto &e : neighbors) {
//         out.push_back(dfs(g, e.target, hybrids, includeInternalNames));
//     }
//
//     std::string res = "(" + out[0];
//
//     for (size_t i = 1; i < out.size(); i++) {
//         res += "," + out[1];
//     }
//     res += ")";
//
//     if (includeInternalNames) {
//         res += nodeID;
//     }
//
//     auto hybrid = hybrids.find(nodeID);
//     if (hybrid != hybrids.end()) {
//         res += hybrid->second;
//     }
//
//     return res;
// }

void openENWK(Graph &g, const std::string &file) {
    std::ifstream f(file);

    if (!f.is_open()) {
        std::cerr << "Couldn't open `" << file << "`." << std::endl;
        std::exit(1);
    }

    std::vector<Token> tokens = tokenize(f);
    f.close();

    if (!parse(g, tokens)) {
        std::cerr << "'" << file << "' is not a valid Extended Newick file." << std::endl;
        std::exit(1);
    }
}

void saveENWK(const Graph &g, const std::string &filename) {
    // std::cout << "\nSaving as ENewick" << std::endl;
    //
    // std::vector<std::string> roots = getRoot(g);
    //
    // if (roots.empty()) {
    //     std::cerr << "Failed to save to '" << filename << "'. Graph has no root."<< std::endl;
    //     std::exit(1);
    // }
    //
    // std::unordered_map<std::string, std::string> hybrids = getHybrids(g);
    //
    // std::string res = dfs(g, roots[0], hybrids, includeInternalNames);
    // std::cout << res << ";" << std::endl;
}
