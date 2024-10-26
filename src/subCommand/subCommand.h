#pragma once

#include <string>

struct SubCommand {
    std::string name;
    std::string desc;
    void (*func)(int, char **);
};

const size_t NUM_SUB_COMMNADS = 3;

extern SubCommand subCommands[NUM_SUB_COMMNADS];
