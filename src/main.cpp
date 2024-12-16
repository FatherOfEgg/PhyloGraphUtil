#include <cstring>
#include <iostream>
#include <string>

#include "formats/format.h"
#include "subCommand/subCommand.h"

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
    for (const SubCommand &sc : subCommands) {
        std::cout << "\t" << sc.name;
        std::cout << "\t" << sc.desc;
        std::cout << std::endl;
    }
    std::cout << std::endl;

    printFormats();
}

int main(int argc, char **argv) {
    if (argc == 1) {
        printUsage();
        return 1;
    }

    if (!strcmp(argv[1], "-h")) {
        printUsage();
        return 0;
    }

    for (const SubCommand &sc : subCommands) {
        if (!sc.name.compare(argv[1])) {
            sc.func(argc - 2, &argv[2]);
            return 0;
        }
    }

    std::cout << "'" << argv[1] << "' is not a valid subcommand/flag." << std::endl;
    std::cout << std::endl;
    printUsage();

    return 1;
}
