#include<string>
#include "gml.h"

int main(int argc, char **argv) {
    GMLGraph gml;
    gml.open(argv[1]);
    gml.print();
}
