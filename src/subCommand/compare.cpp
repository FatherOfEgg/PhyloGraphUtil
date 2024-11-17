#include "compare.h"

#include <cstring>
#include <iostream>

#include "../formats/format.h"
#include "compare/robinsonFoulds.h"

static void compareUsage() {
    std::cout << "PhyloGraphUtil compare" << std::endl;
    std::cout << "Compares one graph to another using Robinson Foulds distance." << std::endl;
    std::cout << std::endl;
    std::cout << "USAGE:" << std::endl;
    std::cout << "\tPhyloGraphUtil convert <INPUT1> <INPUT2>" << std::endl;
    std::cout << std::endl;
    std::cout << "FLAGS:" << std::endl;
    std::cout << "\t-h\tPrints help information." << std::endl;
    std::cout << std::endl;
    printFormats();
}

void compare(int argc, char **argv) {
    if (argc == 0) {
        compareUsage();
        std::exit(1);
    }

    Graph g1 = {.format = FormatType::INVALID};
    Graph g2 = {.format = FormatType::INVALID};

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            compareUsage();
            std::exit(0);
        } else if (g1.format == FormatType::INVALID) {
            g1.open(argv[i]);
        } else if (g2.format == FormatType::INVALID) {
            g2.open(argv[i]);
        }
    }

    if (g1.format == FormatType::INVALID
    ||  g2.format == FormatType::INVALID) {
        compareUsage();
        std::exit(1);
    }

    robinsonFoulds(g1, g2);
}
