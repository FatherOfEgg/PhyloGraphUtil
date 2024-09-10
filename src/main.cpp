#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "eNewick.h"
#include "gml.h"
#include "compare.h"

#define ARRAY_LENGTH(ARR) (sizeof(ARR) / sizeof((ARR)[0]))

const std::string formatStrings[] = {
    "GML",
    "ENWK",
};

struct SubCommand {
    std::string name;
    void (*func)(int, char **);
};

static bool isValidFormat(const std::string &format) {
    auto it = std::find(std::begin(formatStrings), std::end(formatStrings), format);
    return it != std::end(formatStrings);
}

static void printFormats() {
    std::cout << "FORMATS:" << std::endl;
    std::cout << "\t";

    for (size_t i = 0; i < ARRAY_LENGTH(formatStrings); i++) {
        std::cout << formatStrings[i];

        if (i < ARRAY_LENGTH(formatStrings) - 1) {
            std::cout << ", ";
        }
    }

    std::cout << "." << std::endl;
}

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

static void printUsage() {
    std::cout << "PhyloGraphUtil" << std::endl;
    std::cout << std::endl;
    std::cout << "USAGE:" << std::endl;
    std::cout << "\tPhyloGraphUtil <SUBCOMMAND>" << std::endl;
    std::cout << std::endl;
    std::cout << "FLAGS:" << std::endl;
    std::cout << "\t-h\tPrints help information for this and each subcommand." << std::endl;
    std::cout << std::endl;
    std::cout << "SUBCOMMANDS:" << std::endl;
    std::cout << "\tconvert\tConverts from one graph format to another." << std::endl;
    std::cout << "\tprint\tPrints basic info of the read-in graph(s)." << std::endl;
}

static void printConvertUsage() {
    std::cout << "PhyloGraphUtil Convert" << std::endl;
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

static void subcommandConvert(int argc, char **argv) {
    if (argc == 0) {
        printConvertUsage();
        std::exit(1);
    }

    bool includeInternalNames = false;
    std::string format1;
    std::string input;
    std::string format2;
    std::string output;

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            printConvertUsage();
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
                printConvertUsage();
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
        printConvertUsage();
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

static void printCompareUsage() {

}

static void subcommandCompare(int argc, char **argv) {
    if (argc == 0) {
        printCompareUsage();
        std::exit(1);
    }

    std::string format1;
    std::string input1;
    std::string format2;
    std::string input2;

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            printCompareUsage();
            std::exit(0);
        } else if (format1.empty()) {
            if (!isValidFormat(argv[i])) {
                std::cerr << "'" << argv[i] << "' is not a valid format." << std::endl;
                std::exit(1);
            }

            format1 = argv[i];

            if (i + 1 >= argc) {
                std::cerr << "Did not supply a '" << argv[i] << "' file." << std::endl;
                printCompareUsage();
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
                printCompareUsage();
                std::exit(1);
            }

            i++;
            input2 = argv[i];
        }
    }

    if (format1.empty() || input1.empty()
    ||  format2.empty() || input2.empty()) {
        printCompareUsage();
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

    std::cout << "Smallest RF Distance: " << compare(g1, g2) << std::endl;
}

static void printPrintUsage() {
    std::cout << "PhyloGraphUtil Print" << std::endl;
    std::cout << "Prints basic info of the read-in graph(s)." << std::endl;
    std::cout << std::endl;
    std::cout << "USAGE:" << std::endl;
    std::cout << "\tPhyloGraphUtil print <FORMAT 1> <FILE 1> <FORMAT2>..." << std::endl;
    std::cout << std::endl;
    std::cout << "FLAGS:" << std::endl;
    std::cout << "\t-h\tPrints help information." << std::endl;
    std::cout << std::endl;
    printFormats();
}

static void subcommandPrint(int argc, char **argv) {
    if (argc == 0) {
        printPrintUsage();
        std::exit(1);
    }

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            printPrintUsage();
            std::exit(0);
        }

        if (!isValidFormat(argv[i])) {
            std::cerr << "'" << argv[i] << "' is not a valid format." << std::endl;
            std::exit(1);
        }

        if (i + 1 >= argc) {
            std::cerr << "Did not supply a '" << argv[i] << "' file." << std::endl;
            printPrintUsage();
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

int main(int argc, char **argv) {
    if (argc == 1) {
        printUsage();
        return 1;
    }

    const SubCommand subCommands[] = {
        {"convert", subcommandConvert},
        {"compare", subcommandCompare},
        {"print", subcommandPrint},
    };

    if (!strcmp(argv[1], "-h")) {
        printUsage();
        return 0;
    }

    for (size_t i = 0; i < ARRAY_LENGTH(subCommands); i++) {
        if (!subCommands[i].name.compare(argv[1])) {
            subCommands[i].func(argc - 2, &argv[2]);
            return 0;
        }
    }

    std::cout << "'" << argv[1] << "' is not a valid subcommand/flag." << std::endl;
    std::cout << std::endl;
    printUsage();

    return 1;
}
