#include "graph.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "formats/format.h"

void Graph::addNode() {
    adjList.emplace_back();
}

unsigned int Graph::getNumNodes() const {
    return adjList.size();
}

void Graph::addEdge(uint64_t source, uint64_t target) {
    if (source >= adjList.size()) {
        addNode();
    }

    adjList[source].push_back(target);
}

unsigned int Graph::getNumEdges() const {
    unsigned int total = 0;

    for (const auto &e : adjList) {
        total += e.size();
    }

    return total;
}

static uint64_t getRoot(const std::vector<std::vector<uint64_t>> &adjList) {
    std::vector<uint64_t> inDegree(adjList.size());

    for (const auto &n : adjList) {
        for (const auto &t : n) {
            inDegree[t]++;
        }
    }

    auto it = std::find(inDegree.begin(), inDegree.end(), 0);

    if (it == inDegree.end()) {
        std::cerr << "Couldn't find root." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    return std::distance(inDegree.begin(), it);
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

static std::string getExtension(const std::string &filename) {
    size_t pos = filename.find_last_of('.');
    
    if (pos == std::string::npos) {
        return "";
    }
    
    return filename.substr(pos);
}

void Graph::open(const std::string &file) {
    std::string ext = getExtension(file);
    filename = file;

    // Go through each format and their expected extension name(s).
    for (const Format &f : formats) {
        for (const std::string &e : f.exts) {
            if (ext == e) {
                if (!f.open(*this, file)) {
                    std::cerr << "Failed to open `" << file << "`" << std::endl;
                    std::exit(EXIT_FAILURE);
                }

                format = f.type;
                root = getRoot(adjList);
                return;
            }
        }
    }

    // If the file didn't have a valid file extension
    // Try to open it by bruteforcing it with every format
    for (const auto &f : formats) {
        if (f.open(*this, file)) {
            format = f.type;
            root = getRoot(adjList);
            return;
        }
    }

    std::cerr << "Couldn't open '" << file << "'" << std::endl;
    printFormats();
    std::exit(EXIT_FAILURE);
}

void Graph::save(FormatType f, const std::string &filename) const {
    std::string out = filenameNoExt(filename);
    out += formats[static_cast<size_t>(f)].exts[0];
    formats[static_cast<size_t>(f)].save(*this, out);
}

void Graph::print() const {
    for (size_t i = 0; i < adjList.size(); i++) {
        std::cout << i;

        if (reticulations.find(i) != reticulations.end()) {
            std::cout << "r";
        } else {
            std::cout << " ";
        }

        std::cout << ": [";
        for (const auto &t : adjList[i]) {
            std::cout << t << ", ";
        }
        std::cout << "]" << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Reticulations: [";
    for (const auto &p : reticulations) {
        std::cout << p.first << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "Leaf nodes: [" << std::endl;
    for (const auto &p : leafName) {
        std::cout << "    " << p.first << ": " << p.second << "," << std::endl;
    }
    std::cout << "]" << std::endl;

    std::cout << std::endl;

    std::cout << "NumLeaves: " << leaves.size() << std::endl;
    std::cout << "NumReticulations: " << reticulations.size() << std::endl;
    std::cout << "NumNodes: " << getNumNodes() << std::endl;
    std::cout << "NumEdges: " << getNumEdges() << std::endl;
}
