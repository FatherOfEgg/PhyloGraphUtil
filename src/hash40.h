// This is from Super Smash Bros. Ultimate
// It's basically just a crc32 with the length
// of the string attached to the front.
// len + crc32hash
#pragma once

#include <cstdint>
#include <string>

uint64_t hash40(const std::string &str);
