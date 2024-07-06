#include<string>
#include "eNewick.h"
#include "gml.h"

int main(int argc, char **argv) {
    GMLGraph gml;
    gml.open(argv[1]);
    gml.print();

    ENewickGraph ewk(gml);
    ewk.save("testout.ewk");
}
