// ============================================================================
//  wal.h  —  Write-Ahead Log
//
//  OWNER: Yashila (WAL and crash recovery, Milestone 1 Track B).
//
//  WHAT THIS IS:
//  Before we ever tell a client "your write succeeded", we append that write
//  to this log file and flush it to disk. If the process crashes, the file
//  survives. On restart we replay the file to rebuild in-memory state.
//
//  ON-DISK FORMAT (one entry per write, appended in order):
//
//     [ length : 4 bytes ][ crc32 : 4 bytes ][ payload : `length` bytes ]
//
//  The payload is one serialized Record:
//     [ timestamp : 8 ][ tombstone : 1 ][ key_len : 4 ][ key ]
//                                       [ value_len : 4 ][ value ]
//
//  All multi-byte integers are little-endian. crc32 covers the payload only.
//
//  >>> CHECKPOINT NOTE (agree with Naveen) <<<
//  docs/storage-record-format.md lists the WAL fields as a SET of
//  requirements (length, checksum, key_len, value_len, timestamp, tombstone,
//  key, value) but not a fixed byte ORDER. This file commits to the order
//  above. Confirm the SSTable code uses the SAME order so the two agree.
// ============================================================================
#pragma once

#include "common/record.h"
#include <string>
#include <vector>
#include <cstdio>

namespace quorumdb {

// ---------------------------------------------------------------------------
//  WalWriter  —  opens (or creates) a log file and appends records to it.
// ---------------------------------------------------------------------------
class WalWriter {
public:
    // Opens `path` for appending. Creates it if it doesn't exist.
    // Throws std::runtime_error if the file can't be opened.
    explicit WalWriter(const std::string& path);

    // Closes the file (flushing first). Safe to call more than once.
    ~WalWriter();

    // Serializes `record`, prepends length + crc, appends it, and forces it
    // to disk so it survives a crash. Returns only after the bytes are durable.
    void append(const Record& record);

    // Non-copyable: two writers to the same open file would corrupt it.
    WalWriter(const WalWriter&) = delete;
    WalWriter& operator=(const WalWriter&) = delete;

private:
    std::FILE* file_;
    std::string path_;
};

// ---------------------------------------------------------------------------
//  WalReader  —  reads a log file back, one record at a time, for replay.
// ---------------------------------------------------------------------------
class WalReader {
public:
    // Opens `path` for reading. Throws std::runtime_error if it can't open.
    explicit WalReader(const std::string& path);

    ~WalReader();

    // Reads and returns EVERY valid record in order.
    //
    // Stops at the first entry that is torn or fails its checksum, and does
    // NOT include that entry or anything after it. This is correct because a
    // crash can only ever tear the LAST write in an append-only log, and that
    // write was never acknowledged to the client.
    std::vector<Record> readAll();

    WalReader(const WalReader&) = delete;
    WalReader& operator=(const WalReader&) = delete;

private:
    std::FILE* file_;
    std::string path_;
};

} // namespace quorumdb
