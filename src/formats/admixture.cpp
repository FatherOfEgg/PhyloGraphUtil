#include "admixture.h"
#include "../graph.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

enum class TokenType {
    ID,
    LEFT,
    RIGHT,
    TYPE,
};

struct Token {
    TokenType type;
    std::string value;
};

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

        for (const auto &w : words) {
            std::cout << w << ", ";
        }
        std::cout << std::endl;
    }

    return tokens;
}

static bool parse(Graph &g, const std::vector<Token> &tokens) {
    return true;
}

bool openADMIX(Graph &g, const std::string &file) {
    std::ifstream f(file);

    if (!f.is_open()) {
        return false;
    }

    std::vector<Token> tokens = tokenize(f);
    f.close();

    std::exit(0);

    return parse(g, tokens);
}

void saveADMIX(const Graph &g, const std::string &filename) {

}
