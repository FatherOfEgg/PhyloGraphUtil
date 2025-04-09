#pragma once

#include <cstddef>
#include <cstdint>

#define BITS_PER_MASK 64
#define BITMASK_SIZE 2

class Bitmask {
public:
    Bitmask();
    Bitmask(const Bitmask &other);

    bool operator==(const Bitmask &other) const;
    Bitmask &operator|=(const Bitmask &other);

    void setBit(uint64_t bit);

    const uint64_t *getBitmask() const;

    uint64_t countBits() const;

private:
    uint64_t mBitmask[BITMASK_SIZE];
};

struct BitmaskHash {
    size_t operator()(const Bitmask &bm) const;
};
