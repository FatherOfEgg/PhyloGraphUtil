#include "clusterTable.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

ClusterTable::ClusterTable(const Graph &g, const PSW &psw) {
    uint64_t leafCode = 0;
    internalLabels.reserve(g.leafName.size());

    uint64_t rightLeaf = 0;

    for (size_t i = 0; i < psw.size(); i++) {
        const std::pair<uint64_t, uint64_t> p = psw[i];

        // If leaf
        if (p.second == 0) {
            std::string leafName = g.leafName.at(p.first);
            internalLabels[leafName] = leafCode;

            rightLeaf = leafCode;
            leafCode++;
        } else {
            uint64_t leftLeafIndex = psw[i - p.second].first;
            std::string leafName = g.leafName.at(leftLeafIndex);
            uint64_t leftLeaf = internalLabels[leafName];

            ct[leftLeaf].insert(rightLeaf);
        }
    }

}

uint64_t ClusterTable::encode(const std::string &l) const {
    return internalLabels.at(l);
}

bool ClusterTable::isClust(uint64_t L, uint64_t R) const {
    auto itL = ct.find(L);

    if (itL == ct.end()) {
        return false;
    }

    return itL->second.find(R) != itL->second.end();
}
