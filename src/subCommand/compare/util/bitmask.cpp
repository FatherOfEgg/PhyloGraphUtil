#include "bitmask.h"

#include <cassert>
#include <cstdint>
#include <functional>

Bitmask::Bitmask()
: mBitmask{0} {}

Bitmask::Bitmask(const Bitmask &other) {
    for (size_t i = 0; i < BITMASK_SIZE; i++) {
        mBitmask[i] = other.mBitmask[i];
    }
}

bool Bitmask::operator==(const Bitmask &other) const {
    for (size_t i = 0; i < BITMASK_SIZE; i++) {
        if (mBitmask[i] != other.mBitmask[i]) {
            return false;
        }
    }

    return true;
}

Bitmask &Bitmask::operator|=(const Bitmask &other) {
    for (size_t i = 0; i < BITMASK_SIZE; i++) {
        this->mBitmask[i] |= other.mBitmask[i];
    }

    return *this;
}

void Bitmask::setBit(uint64_t bit) {
    size_t index = bit / BITS_PER_MASK;
    size_t offset = bit % BITS_PER_MASK;

    assert(index < BITMASK_SIZE);

    mBitmask[index] |= (1UL << offset);
}

const uint64_t *Bitmask::getBitmask() const {
    return mBitmask;
}

uint64_t Bitmask::countBits() const {
    uint64_t count = 0;

    // Brian Kernighan's Algorithm
    for (size_t i = 0; i < BITMASK_SIZE; i++) {
        uint64_t v = mBitmask[i];

        while (v) {
            v &= (v - 1);
            count++;
        }
    }

    return count;
}

size_t BitmaskHash::operator()(const Bitmask &bm) const {
    size_t hash = 0;

    for (size_t i =0; i < BITMASK_SIZE; i++) {
        // Taken from https://www.boost.org/doc/libs/1_87_0/libs/container_hash/doc/html/hash.html#notes_hash_combine
        hash ^= std::hash<uint64_t>{}(bm.getBitmask()[i]) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }

    return hash;
}
