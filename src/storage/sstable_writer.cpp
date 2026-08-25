// ============================================================================
//  sstable_writer.cpp  —  implementation of SSTableWriter (see header).
//  OWNER: Yashila.
// ============================================================================
#include "storage/sstable_writer.h"
#include "storage/sstable_reader.h"   // reuse Naveen's sstable_checksum()

#include <array>
#include <cstdint>
#include <fstream>

namespace quorumdb {
namespace {

// The 4-byte file signature. MUST match kMagic in sstable_reader.cpp.
constexpr std::array<char, 4> kMagic{'Q', 'D', 'B', '1'};

// --- little-endian writers (mirror the reader's read_u32 / read_u64) --------
void put_u32(std::string& out, std::uint32_t v) {
    out.push_back(static_cast<char>( v        & 0xFFU));
    out.push_back(static_cast<char>((v >>  8) & 0xFFU));
    out.push_back(static_cast<char>((v >> 16) & 0xFFU));
    out.push_back(static_cast<char>((v >> 24) & 0xFFU));
}

void put_u64(std::string& out, std::uint64_t v) {
    for (int i = 0; i < 8; ++i)
        out.push_back(static_cast<char>((v >> (8 * i)) & 0xFFU));
}

// Serialize one record in the EXACT field order the reader expects:
//   key_size, value_size, timestamp, tombstone, key, value, checksum
void append_entry(std::string& out, const Record& r) {
    put_u32(out, static_cast<std::uint32_t>(r.key.size()));
    put_u32(out, static_cast<std::uint32_t>(r.value.size()));
    put_u64(out, r.timestamp);
    out.push_back(r.tombstone ? 1 : 0);
    out.append(r.key);
    out.append(r.value);
    // Reuse Naveen's checksum so the two implementations can never disagree.
    put_u32(out, sstable_checksum(r));
}

// Write a full buffer to `path`. Returns false if anything fails.
bool write_buffer_to_file(const std::filesystem::path& path,
                          const std::string& buffer) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    out.flush();
    return static_cast<bool>(out);   // false if any write/flush failed
}

}  // namespace

// ---------------------------------------------------------------------------
//  Map overload: the normal flush path. A std::map is always sorted+unique,
//  so we can serialize its records directly with no ordering checks.
// ---------------------------------------------------------------------------
bool SSTableWriter::write(const std::filesystem::path& path,
                          const Memtable::Records& records) {
    std::string buffer;
    buffer.append(kMagic.data(), kMagic.size());   // magic header, once

    for (const auto& [key, record] : records) {
        (void)key;                                 // key is record.key already
        append_entry(buffer, record);
    }

    return write_buffer_to_file(path, buffer);
}

// ---------------------------------------------------------------------------
//  Vector overload: for tests / callers holding a plain vector. We DON'T trust
//  the caller to have sorted it, so we verify strictly-ascending unique keys
//  first. If the check fails we write nothing and return false, catching the
//  mistake here rather than producing a file the reader would reject.
// ---------------------------------------------------------------------------
bool SSTableWriter::write(const std::filesystem::path& path,
                          const std::vector<Record>& sorted_records) {
    for (std::size_t i = 1; i < sorted_records.size(); ++i) {
        if (!(sorted_records[i - 1].key < sorted_records[i].key)) {
            return false;   // out of order, or a duplicate key
        }
    }

    std::string buffer;
    buffer.append(kMagic.data(), kMagic.size());
    for (const Record& r : sorted_records) {
        append_entry(buffer, r);
    }

    return write_buffer_to_file(path, buffer);
}

}  // namespace quorumdb
