#include "print.h"

#include <iostream>

#include "../graph.h"

static void printUsage() {
    std::cout << "PhyloGraphUtil print" << std::endl;
    std::cout << "Prints basic info of the read-in graph(s)." << std::endl;
    std::cout << std::endl;
    std::cout << "USAGE:" << std::endl;
    std::cout << "\tPhyloGraphUtil print <FORMAT 1> <FILE 1> <FORMAT2>..." << std::endl;
    std::cout << std::endl;
    std::cout << "FLAGS:" << std::endl;
    std::cout << "\t-h\tPrints help information." << std::endl;
    std::cout << std::endl;
    // printFormats();
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

        // if (!isValidFormat(argv[i])) {
        //     std::cerr << "'" << argv[i] << "' is not a valid format." << std::endl;
        //     std::exit(1);
        // }

        if (i + 1 >= argc) {
            std::cerr << "Did not supply a '" << argv[i] << "' file." << std::endl;
            printUsage();
            std::exit(1);
        }

        Graph g;

        if (!strcmp(argv[i], "GML")) {
            openGML(g, argv[i + 1]);
        } else if (!strcmp(argv[i], "ENWK")) {
            openENWK(g, argv[i + 1]);
        }

        i++;

        std::cout << "'" << argv[i] << "':" << std::endl;
        g.print();
        std::cout << std::endl;
    }
}
