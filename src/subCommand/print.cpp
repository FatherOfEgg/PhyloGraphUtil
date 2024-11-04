#include "print.h"

#include <cstring>
#include <iostream>

#include "../graph.h"
#include "../formats/format.h"

static void printUsage() {
    std::cout << "PhyloGraphUtil print" << std::endl;
    std::cout << "Prints basic info of the read-in graph(s)." << std::endl;
    std::cout << std::endl;
    std::cout << "USAGE:" << std::endl;
    std::cout << "\tPhyloGraphUtil print <FILE 1> <FILE 2>..." << std::endl;
    std::cout << std::endl;
    std::cout << "FLAGS:" << std::endl;
    std::cout << "\t-h\tPrints help information." << std::endl;
    std::cout << std::endl;
    printFormats();
}

void print(int argc, char **argv) {
    if (argc == 0) {
        printUsage();
        std::exit(1);
    }

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            printUsage();
            std::exit(0);
        }

        Graph g;

        g.open(argv[i]);

        std::cout << "'" << argv[i] << "':" << std::endl;
        g.print();
        std::cout << std::endl;
    }
}
