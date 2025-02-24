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

    for (const Format &f : formats) {
        std::cout << "\t" << f.name << "\t";

        std::cout << f.exts[0];
        for (size_t i = 1; i < f.exts.size(); i++) {
            if (!f.exts[i].empty()) {
                std::cout << ", " << f.exts[i];
            }
        }

        std::cout << std::endl;
    }
}
