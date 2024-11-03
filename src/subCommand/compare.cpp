#include "compare.h"

#include <iostream>

#include "../formats/format.h"

static void compareUsage() {
    std::cout << "PhyloGraphUtil compare" << std::endl;
    std::cout << "Compares one graph to another using Robinson Foulds distance." << std::endl;
    std::cout << std::endl;
    std::cout << "USAGE:" << std::endl;
    std::cout << "\tPhyloGraphUtil convert <FORMAT 1> <INPUT1> <FORMAT2> <INPUT2>" << std::endl;
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

    std::string format1;
    std::string input1;
    std::string format2;
    std::string input2;

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            compareUsage();
            std::exit(0);
        } else if (format1.empty()) {
            if (!isValidFormat(argv[i])) {
                std::cerr << "'" << argv[i] << "' is not a valid format." << std::endl;
                std::exit(1);
            }

            format1 = argv[i];

            if (i + 1 >= argc) {
                std::cerr << "Did not supply a '" << argv[i] << "' file." << std::endl;
                compareUsage();
                std::exit(1);
            }

            i++;
            input1 = argv[i];
        } else if (format2.empty()) {
            if (!isValidFormat(argv[i])) {
                std::cerr << "'" << argv[i] << "' is not a valid format." << std::endl;
                std::exit(1);
            }

            format2 = argv[i];

            if (i + 1 >= argc) {
                std::cerr << "Did not supply a '" << argv[i] << "' file." << std::endl;
                compareUsage();
                std::exit(1);
            }

            i++;
            input2 = argv[i];
        }
    }

    if (format1.empty() || input1.empty()
    ||  format2.empty() || input2.empty()) {
        compareUsage();
        std::exit(1);
    }

    Graph g1;
    Graph g2;

    if (format1 == "GML") {
        openGML(g1, input1);
    } else if (format1 == "ENWK") {
        openENWK(g1, input1);
    }

    if (format2 == "GML") {
        openGML(g2, input2);
    } else if (format2 == "ENWK") {
        openENWK(g2, input2);
    }

    compare(g1, g2);
}
