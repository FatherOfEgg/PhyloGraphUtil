#include "convert.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "../formats/format.h"

static void convertUsage() {
    std::cout << "PhyloGraphUtil convert" << std::endl;
    std::cout << "Converts from one graph format to another." << std::endl;
    std::cout << std::endl;
    std::cout << "USAGE:" << std::endl;
    std::cout << "\tPhyloGraphUtil convert <INPUT> <FORMAT OUT> [OUTPUT]" << std::endl;
    std::cout << std::endl;
    std::cout << "FLAGS:" << std::endl;
    std::cout << "\t-h\tPrints help information." << std::endl;
    // std::cout << "\t-i\tIf converting to ENWK, including this flag will include internal names." << std::endl;
    std::cout << std::endl;
    std::cout << "OPTIONAL:" << std::endl;
    std::cout << "\tYou can supply a 3rd argument to specify the directory/name of the output." << std::endl;
    std::cout << "\tDefaults to <INPUT>, but with <FORMAT OUT>'s extension." << std::endl;
    std::cout << std::endl;
    printFormats();
}

void convert(int argc, char **argv) {
    if (argc == 0) {
        convertUsage();
        std::exit(EXIT_FAILURE);
    }

    // bool includeInternalNames = false;
    Graph g = {.format = FormatType::INVALID};

    std::string input;
    FormatType formatOut = FormatType::INVALID;
    std::string filename;

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            convertUsage();
            std::exit(EXIT_SUCCESS);
        // } else if (!strcmp(argv[i], "-i")) {
        //     includeInternalNames = true;
        } else if (g.format == FormatType::INVALID) {
            g.open(argv[i]);
            input = argv[i];
        } else if (formatOut == FormatType::INVALID) {
            std::string formatIn = argv[i];

            std::transform(
                formatIn.begin(),
                formatIn.end(),
                formatIn.begin(),
                [](char c) {
                    return std::toupper(c);
                }
            );

            for (const Format &f : formats) {
                if (f.name == formatIn) {
                    formatOut = f.type;
                    break;
                }
            }
        } else if (filename.empty()) {
            filename = argv[i];
        }
    }

    if (g.format == FormatType::INVALID
    ||  formatOut == FormatType::INVALID
    ||  input.empty()) {
        convertUsage();
        std::exit(EXIT_FAILURE);
    }

    if (g.format == formatOut) {
        std::cout << "No need for conversion, b/c you're converting between 2 formats that are the same." << std::endl;
        std::exit(EXIT_SUCCESS);
    }

    if (filename.empty()) {
        filename = input;
    }

    g.save(formatOut, filename);
}
