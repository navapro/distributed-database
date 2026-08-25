// ============================================================================
//  crc32.h  —  a tiny, self-contained CRC32 checksum.
//
//  OWNER: Yashila (shared storage utility).
//
//  You do NOT need to understand the bit-twiddling below. The only thing that
//  matters conceptually: crc32(bytes) turns a chunk of bytes into a 4-byte
//  number, and the SAME bytes always produce the SAME number. If even one bit
//  of the bytes changes, the number changes. That's how we detect corruption.
//
//  This is the standard "IEEE" CRC32 (same variety used by zlib/gzip).
// ============================================================================
#pragma once

#include <cstdint>
#include <cstddef>

namespace quorumdb {

inline uint32_t crc32(const void* data, size_t length) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint32_t crc = 0xFFFFFFFFu;              // standard starting value

    for (size_t i = 0; i < length; ++i) {
        crc ^= bytes[i];                     // mix in the next byte
        for (int bit = 0; bit < 8; ++bit) {
            // If the low bit is set, shift and XOR with the CRC polynomial.
            // Otherwise just shift. This is the textbook bitwise CRC loop.
            uint32_t mask = -(crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }

    return ~crc;                             // final inversion, also standard
}

} // namespace quorumdb
