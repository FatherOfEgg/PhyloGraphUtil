#include "subCommand.h"

#include "compare.h"
#include "convert.h"
#include "print.h"

SubCommand subCommands[NUM_SUB_COMMNADS] = {
    {
        "convert",
        "Converts from one graph format to another.",
        convert
    },
    {
        "compare",
        "Compares one graph to another using Robinson Foulds distance.",
        compare
    },
    {
        "print",
        "Prints basic info of the read-in graph(s).",
        print
    },
};
