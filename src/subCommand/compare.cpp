#include "compare.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "../formats/format.h"
#include "compare/jaccardIndex.h"
#include "compare/robinsonFoulds.h"

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
    std::cout << "\tji\tJaccard index." << std::endl;
    std::cout << std::endl;
    printFormats();
}

void compare(int argc, char **argv) {
    if (argc == 0) {
        compareUsage();
        std::exit(EXIT_FAILURE);
    }

    if (!strcmp(argv[0], "-h")) {
        compareUsage();
        std::exit(EXIT_SUCCESS);
    }

    Graph g1 = {.format = FormatType::INVALID};
    Graph g2 = {.format = FormatType::INVALID};

    g1.open(argv[1]);
    g2.open(argv[2]);

    if (g1.format == FormatType::INVALID
    ||  g2.format == FormatType::INVALID) {
        compareUsage();
        std::exit(EXIT_FAILURE);
    }

    if (!strcmp(argv[0], "rf")) {
        robinsonFoulds(g1, g2);
    } else if (!strcmp(argv[0], "ji")) {
        jaccardIndex(g1, g2);
    } else {
        std::cout << "'" << argv[0] << "' is not a valid method." << std::endl;
        compareUsage();
        std::exit(EXIT_SUCCESS);
    }
}
