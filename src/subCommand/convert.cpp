#include "convert.h"

#include <algorithm>
#include <iostream>
#include <string>

#include "../formats/format.h"

static std::string filenameNoExt(const std::string &file) {
    size_t dotPos = file.find_last_of(".");

    if (dotPos == std::string::npos) {
        return file;
    }

    size_t lastSlash = file.find_last_of("/\\");

    if (dotPos > lastSlash) {
        return file.substr(0, dotPos);
    } else {
        return file;
    }
}

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
        std::exit(1);
    }

    // bool includeInternalNames = false;
    Graph g = {.format = FormatType::INVALID};

    std::string input;
    FormatType formatOut = FormatType::INVALID;
    std::string filename;

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            convertUsage();
            std::exit(0);
        // } else if (!strcmp(argv[i], "-i")) {
        //     includeInternalNames = true;
        } else if (g.format == FormatType::INVALID) {
            g.open(argv[i]);
            input = argv[i];
        } else if (formatOut == FormatType::INVALID) {
            for (const Format &f : formats) {
                if (f.name == argv[i]) {
                    formatOut = f.format;
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
        std::exit(1);
    }

    if (g.format == formatOut) {
        std::cout << "No need for conversion, b/c you're converting between 2 formats that are the same." << std::endl;
        std::exit(0);
    }

    if (filename.empty()) {
        std::string ext = formats[static_cast<size_t>(formatOut)].name;
        std::transform(
            ext.begin(),
            ext.end(),
            ext.begin(),
            [](char c) {
                return std::tolower(c);
            }
        );

        filename = filenameNoExt(input) + "." + ext;
    }

    g.save(formatOut, filename);
}
