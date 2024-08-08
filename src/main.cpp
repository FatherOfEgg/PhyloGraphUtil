#include <cstring>
#include <iostream>
#include <string>

#include "eNewick.h"
#include "gml.h"

static void printUsage() {
    std::cout << "Description:" << std::endl;
    std::cout << "\tAttempts to convert from GML to Extended Newick, and vice versa." << std::endl;
    std::cout << std::endl;
    std::cout << "USAGE:" << std::endl;
    std::cout << "\tPhyloGraphConverter <FLAG> <INPUT>" << std::endl;
    std::cout << std::endl;
    std::cout << "FLAGS:" << std::endl;
    std::cout << "\t-h\tPrints help information." << std::endl;
    std::cout << "\t-p\tPrints out the parsed graph, and converts." << std::endl;
    std::cout << "\t-P\tOnly prints out the parsed graph." << std::endl;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    bool justPrint = false;
    bool print = false;
    std::string input;
    bool isGML = true;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            printUsage();
            return 0;
        } else if (!strcmp(argv[i], "-p")) {
            print = true;
        } else if (!strcmp(argv[i], "-P")) {
            justPrint = true;
        } else if (input.empty()) {
            input = argv[i];
        }
    }

    if (input.empty()) {
        std::cout << "Please supply a file." << std::endl;
        printUsage();
        return 1;
    }

    std::string path;
    std::string filename = input;
    size_t lastSlash = input.find_last_of("/\\");

    if (lastSlash != std::string::npos) {
        path = input.substr(0, lastSlash + 1);
        filename = input.substr(lastSlash + 1);
    }

    std::string filename_no_ext = filename;
    std::string ext;
    size_t period = filename.find_last_of('.');

    if (period != std::string::npos) {
        filename_no_ext = filename.substr(0, period);
        ext = filename.substr(period + 1);

        if (ext == "enwk" || ext == "ewk" || ext == "nwk") {
            isGML = false;
        }
    }

    Graph g;

    if (isGML) {
        openGML(g, input);
    } else {
        openENWK(g, input);
    }

    if (print || justPrint) {
        g.print();
        if (justPrint) {
            return 0;
        }
    }

    if (isGML) {
        saveENWK(g, filename_no_ext + ".enwk", false);
    } else {
        saveGML(g, filename_no_ext + ".gml");
    }
}
