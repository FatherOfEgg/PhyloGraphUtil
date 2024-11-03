#include "convert.h"

#include <iostream>

#include "../formats/format.h"

static void convertUsage() {
    std::cout << "PhyloGraphUtil convert" << std::endl;
    std::cout << "Converts from one graph format to another." << std::endl;
    std::cout << std::endl;
    std::cout << "USAGE:" << std::endl;
    std::cout << "\tPhyloGraphUtil convert <FORMAT 1> <INPUT> <FORMAT2> [OUTPUT]" << std::endl;
    std::cout << std::endl;
    std::cout << "FLAGS:" << std::endl;
    std::cout << "\t-h\tPrints help information." << std::endl;
    std::cout << "\t-i\tIf converting to ENWK, including this flag will include internal names." << std::endl;
    std::cout << std::endl;
    std::cout << "OPTIONAL:" << std::endl;
    std::cout << "\tYou can supply a 4th argument to specify the directory/name of the output." << std::endl;
    std::cout << "\tDefaults to <INPUT>, but with <FORMAT2>'s extension." << std::endl;
    std::cout << std::endl;
    printFormats();
}

void convert(int argc, char **argv) {
    if (argc == 0) {
        convertUsage();
        std::exit(1);
    }

    bool includeInternalNames = false;
    std::string format1;
    std::string input;
    std::string format2;
    std::string output;

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            convertUsage();
            std::exit(0);
        } else if (!strcmp(argv[i], "-i")) {
            includeInternalNames = true;
        } else if (format1.empty()) {
            if (!isValidFormat(argv[i])) {
                std::cerr << "'" << argv[i] << "' is not a valid format." << std::endl;
                std::exit(1);
            }

            format1 = argv[i];

            if (i + 1 >= argc) {
                std::cerr << "Did not supply a '" << argv[i] << "' file." << std::endl;
                convertUsage();
                std::exit(1);
            }

            i++;
            input = argv[i];
        } else if (format2.empty()) {
            if (!isValidFormat(argv[i])) {
                std::cerr << "'" << argv[i] << "' is not a valid format." << std::endl;
                std::exit(1);
            }

            format2 = argv[i];
        } else if (output.empty()) {
            output = argv[i];
        }
    }

    if (format1.empty() || input.empty() || format2.empty()) {
        convertUsage();
        std::exit(1);
    }

    if (format1 == format2) {
        std::cout << "No need for conversion, b/c you're converting between 2 formats that are the same." << std::endl;
        std::exit(0);
    }

    if (output.empty()) {
        std::string ext = format2;
        std::transform(
            ext.begin(),
            ext.end(),
            ext.begin(),
            [](char c) {
                return std::tolower(c);
            }
        );

        output = filenameNoExt(input) + "." + ext;
    }

    Graph g;

    if (format1 == "GML") {
        openGML(g, input);
    } else if (format1 == "ENWK") {
        openENWK(g, input);
    }

    if (format2 == "GML") {
        saveGML(g, output);
    } else if (format2 == "ENWK") {
        saveENWK(g, output, includeInternalNames);
    }
}
