// ============================================================================
//  sstable_writer.h  —  writes an immutable, sorted, checksummed SSTable file.
//
//  OWNER: Yashila (SSTable read/write format, Milestone 1 Track C).
//
//  WHAT AN SSTABLE IS:
//  A "Sorted String Table" is a file of records sorted by key that, once
//  written, is NEVER modified. When the in-memory memtable fills up, we flush
//  it into one of these files and start a fresh memtable. Immutability is what
//  makes reads and compaction simple later on.
//
//  THIS WRITER produces files that Naveen's SSTableReader reads. The on-disk
//  format is therefore DICTATED by his reader (src/storage/sstable_reader.cpp).
//  We must match it byte-for-byte, so this file documents that exact format:
//
//    [ magic : 4 bytes = 'Q','D','B','1' ]        (once, at the very start)
//    then for each record, in ascending key order:
//        [ key_size   : u32 little-endian ]
//        [ value_size : u32 little-endian ]
//        [ timestamp  : u64 little-endian ]
//        [ tombstone  : 1 byte, 0 or 1     ]
//        [ key bytes  : key_size bytes      ]
//        [ value bytes: value_size bytes    ]
//        [ checksum   : u32 little-endian   ]   (== sstable_checksum(record))
//    EOF is simply the end of the file after the last record.
//
//  TWO HARD RULES his reader enforces (so this writer enforces them too):
//    1. Keys must be STRICTLY ASCENDING and UNIQUE. His reader rejects the
//       whole file if any key is <= the previous key.
//    2. The checksum must be computed by his sstable_checksum() function
//       (FNV-1a over the fields), NOT the WAL's CRC32. We reuse his function
//       directly so the two can never drift apart.
// ============================================================================
#pragma once

#include "common/record.h"
#include "storage/memtable.h"      // for Memtable::Records (a std::map)

#include <filesystem>
#include <string>
#include <vector>

namespace quorumdb {

class SSTableWriter {
public:
    // Flush a Memtable's records straight to `path`.
    //
    // A Memtable is a std::map<std::string, Record>, so its records are ALREADY
    // sorted and unique by key — exactly what the SSTable format requires. This
    // is the normal path used when a real memtable is flushed to disk.
    //
    // Returns true on success. Returns false (and writes no usable file) if the
    // file can't be opened or a write fails.
    static bool write(const std::filesystem::path& path,
                      const Memtable::Records& records);

    // Convenience overload for tests and callers that hold a plain vector.
    //
    // The vector MUST already be sorted by key with no duplicates. If it isn't,
    // this returns false rather than writing a file the reader would reject —
    // so a caller mistake is caught here instead of silently producing a bad
    // file. (The map overload can't hit this because a map is always sorted.)
    static bool write(const std::filesystem::path& path,
                      const std::vector<Record>& sorted_records);
};

}  // namespace quorumdb
