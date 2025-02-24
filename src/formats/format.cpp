#include "format.h"
#include "formatType.h"

#include <iostream>

#include "gml.h"
#include "eNewick.h"
#include "admixture.h"
#include "treemix.h"

Format formats[static_cast<size_t>(FormatType::NUM)] = {
    {
        FormatType::GML,
        "GML",
        {".gml"},
        openGML,
        saveGML
    },
    {
        FormatType::ENWK,
        "ENWK",
        {".enwk", ".ewk"},
        openENWK,
        saveENWK
    },
    {
        FormatType::ADMIX,
        "ADMIX",
        {".admix"},
        openADMIX,
        saveADMIX
    },
{
        FormatType::TREEMIX,
        "TREEMIX",
        {".treemix"},
        openTreemix,
        saveTreemix
    }
};

void printFormats() {
    std::cout << "FORMATS:" << std::endl;
    std::cout << "\t";

    for (size_t i = 0; i < NUM_FORMATS; i++) {
        std::cout << formats[i].name;

        if (i < NUM_FORMATS - 1) {
            std::cout << ", ";
        }
    }

    std::cout << "." << std::endl;
}
