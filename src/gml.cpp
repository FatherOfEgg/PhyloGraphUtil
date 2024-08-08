#include "gml.h"
#include "graph.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
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

static bool parse(Graph &g, const std::vector<Token> &tokens) {
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

            g.addNode(n);
        } else if (tokens[i].type == TokenType::EDGE) {
            i++;

            if (tokens[i].type != TokenType::OPEN_BRACKET) {
                return false;
            }

            i++;
            Edge e = {.length = std::nan("")};
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

                        bool isNumber = std::all_of(e.label.begin(), e.label.end(), [](const auto &c) {
                            return isdigit(c) || c == '.';
                        });

                        if (isNumber) {
                            e.length = std::stod(e.label);
                        }
                    } else if (attributeName == "length") {
                        e.length = std::stod(tokens[i].value);
                    }
                }

                i++;
            }

            g.addEdge(e);
        }
    }

    return true;
}

void openGML(Graph &g, const std::string &file) {
    std::ifstream f(file);

    if (!f.is_open()) {
        std::cerr << "Couldn't open `" << file << "`." << std::endl;
        std::exit(1);
    }

    std::vector<Token> tokens = tokenize(f);
    f.close();

    if (!parse(g, tokens)) {
        std::cerr << "'" << file << "' is not a valid GML file." << std::endl;
        std::exit(1);
    }
}

void saveGML(Graph &g, const std::string &filename) {
    std::ofstream f(filename);

    if (!f) {
        std::cerr << "Failed to save to '" << filename << "'."<< std::endl;
        std::exit(1);
    }

    std::string indent = "    ";

    f << "graph [" << std::endl;

    f << indent << "directed 1" << std::endl;

    for (const auto &n : g.nodes) {
        f << indent << "node [" << std::endl;
        indent += "    ";

        f << indent << "id " << n.id << std::endl;
        f << indent << "label \"" << n.label << "\"" << std::endl;

        indent = indent.substr(0, indent.length() - 4);
        f << indent << "]" << std::endl;
    }

    for (const auto &e : g.edges) {
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
