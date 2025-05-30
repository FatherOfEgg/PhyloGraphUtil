#include "compare.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "../formats/format.h"
// #include "compare/jaccardIndex.h"
#include "compare/precisionAndRecall.h"
#include "compare/robinsonFoulds.h"

typedef void (*compareFunc)(const Graph &, const Graph &);

struct CompareMethod {
    std::string name;
    compareFunc fn;
};

static CompareMethod compareMethods[] = {
    {"rf", robinsonFoulds},
    // {"ji", jaccardIndex},
    {"pr", precisionAndRecall},
};

static void compareUsage() {
    std::cout << "PhyloGraphUtil compare" << std::endl;
    std::cout << "Compares one graph to another using a specified method." << std::endl;
    std::cout << std::endl;
    std::cout << "USAGE:" << std::endl;
    std::cout << "\tPhyloGraphUtil compare <METHOD> <INPUT1> <INPUT2>" << std::endl;
    std::cout << std::endl;
    std::cout << "FLAGS:" << std::endl;
    std::cout << "\t-h\tPrints help information." << std::endl;
    std::cout << std::endl;
    std::cout << "METHOD:" << std::endl;
    std::cout << "\trf\tRobinson Foulds." << std::endl;
    // std::cout << "\tji\tJaccard index." << std::endl;
    std::cout << "\tpr\tPrecision & Recall. (INPUT1 is original, INPUT2 will be compared to INPUT1" << std::endl;
    std::cout << std::endl;
    printFormats();
}

void compare(int argc, char **argv) {
    if (argc == 0) {
        compareUsage();
        std::exit(EXIT_FAILURE);
    }

    compareFunc cf = nullptr;
    Graph g1 = {.format = FormatType::INVALID};
    Graph g2 = {.format = FormatType::INVALID};

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            compareUsage();
            std::exit(EXIT_SUCCESS);
        }

        if (cf == nullptr) {
            for (const CompareMethod &cm : compareMethods) {
                if (argv[i] == cm.name) {
                    cf = cm.fn;
                    break;
                }
            }

            if (cf == nullptr) {
                std::cout << "'" << argv[i] << "' is not a valid comparison method" << std::endl;
                compareUsage();
                std::exit(EXIT_FAILURE);
            }
        } else if (g1.format == FormatType::INVALID) {
            g1.open(argv[i]);
        } else if (g2.format == FormatType::INVALID) {
            g2.open(argv[i]);
        }
    }

    if (cf == nullptr
    ||  g1.format == FormatType::INVALID
    ||  g2.format == FormatType::INVALID) {
        compareUsage();
        std::exit(EXIT_FAILURE);
    }

    cf(g1, g2);
}
