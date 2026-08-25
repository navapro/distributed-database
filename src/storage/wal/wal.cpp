// ============================================================================
//  wal.cpp  —  implementation of WalWriter and WalReader (see wal.h).
//  OWNER: Yashila.
// ============================================================================
#include "storage/wal/wal.h"
#include "storage/wal/crc32.h"

#include <stdexcept>
#include <cstring>

#if defined(_WIN32)
  #include <io.h>
  #define FSYNC(fd) _commit(fd)
  #define FILENO   _fileno
#else
  #include <unistd.h>          // for fsync()
  #define FSYNC(fd) ::fsync(fd)
  #define FILENO   ::fileno
#endif

namespace quorumdb {

// ---------------------------------------------------------------------------
//  Little-endian integer helpers.
//
//  "Little-endian" just means we store the least-significant byte first. We
//  pick ONE ordering and always use it, so a file written on one machine reads
//  back correctly. These push the bytes of an integer onto a byte buffer, and
//  read them back out.
// ---------------------------------------------------------------------------
namespace {

void putU32(std::string& out, uint32_t v) {
    out.push_back(static_cast<char>( v        & 0xFF));
    out.push_back(static_cast<char>((v >>  8) & 0xFF));
    out.push_back(static_cast<char>((v >> 16) & 0xFF));
    out.push_back(static_cast<char>((v >> 24) & 0xFF));
}

void putU64(std::string& out, uint64_t v) {
    for (int i = 0; i < 8; ++i)
        out.push_back(static_cast<char>((v >> (8 * i)) & 0xFF));
}

uint32_t getU32(const uint8_t* p) {
    return  static_cast<uint32_t>(p[0])
         | (static_cast<uint32_t>(p[1]) <<  8)
         | (static_cast<uint32_t>(p[2]) << 16)
         | (static_cast<uint32_t>(p[3]) << 24);
}

uint64_t getU64(const uint8_t* p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i)
        v |= static_cast<uint64_t>(p[i]) << (8 * i);
    return v;
}

// Turn a Record into its payload bytes (the part after length + crc).
// Layout: timestamp(8) | tombstone(1) | key_len(4) | key | value_len(4) | value
std::string serializeRecord(const Record& r) {
    std::string p;
    putU64(p, r.timestamp);
    p.push_back(r.tombstone ? 1 : 0);
    putU32(p, static_cast<uint32_t>(r.key.size()));
    p.append(r.key);
    putU32(p, static_cast<uint32_t>(r.value.size()));
    p.append(r.value);
    return p;
}

// Turn payload bytes back into a Record. Returns false if the bytes are too
// short to be a valid payload (i.e. torn).
bool deserializeRecord(const std::string& payload, Record& out) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(payload.data());
    size_t n = payload.size();
    size_t off = 0;

    auto need = [&](size_t bytes) { return off + bytes <= n; };

    if (!need(8)) return false;
    out.timestamp = getU64(p + off); off += 8;

    if (!need(1)) return false;
    out.tombstone = (p[off] != 0); off += 1;

    if (!need(4)) return false;
    uint32_t klen = getU32(p + off); off += 4;
    if (!need(klen)) return false;
    out.key.assign(payload, off, klen); off += klen;

    if (!need(4)) return false;
    uint32_t vlen = getU32(p + off); off += 4;
    if (!need(vlen)) return false;
    out.value.assign(payload, off, vlen); off += vlen;

    return true;
}

} // anonymous namespace

// ===========================================================================
//  WalWriter
// ===========================================================================
WalWriter::WalWriter(const std::string& path) : file_(nullptr), path_(path) {
    // "ab" = append, binary. Every write goes to the end of the file.
    file_ = std::fopen(path.c_str(), "ab");
    if (!file_)
        throw std::runtime_error("WalWriter: cannot open " + path);
}

WalWriter::~WalWriter() {
    if (file_) { std::fclose(file_); file_ = nullptr; }
}

void WalWriter::append(const Record& record) {
    // 1. Serialize the record into payload bytes.
    std::string payload = serializeRecord(record);

    // 2. Compute the checksum over the payload.
    uint32_t crc = crc32(payload.data(), payload.size());

    // 3. Build the full entry: length + crc + payload.
    std::string entry;
    putU32(entry, static_cast<uint32_t>(payload.size()));
    putU32(entry, crc);
    entry.append(payload);

    // 4. Write the whole entry in one go.
    size_t written = std::fwrite(entry.data(), 1, entry.size(), file_);
    if (written != entry.size())
        throw std::runtime_error("WalWriter: short write to " + path_);

    // 5. Force it to disk. fflush() pushes it out of our program's buffer;
    //    fsync() forces the operating system to actually write it to the
    //    physical disk. Without this, a crash could lose "acknowledged" data.
    if (std::fflush(file_) != 0)
        throw std::runtime_error("WalWriter: fflush failed on " + path_);
    if (FSYNC(FILENO(file_)) != 0)
        throw std::runtime_error("WalWriter: fsync failed on " + path_);
}

// ===========================================================================
//  WalReader
// ===========================================================================
WalReader::WalReader(const std::string& path) : file_(nullptr), path_(path) {
    file_ = std::fopen(path.c_str(), "rb");   // read, binary
    if (!file_)
        throw std::runtime_error("WalReader: cannot open " + path);
}

WalReader::~WalReader() {
    if (file_) { std::fclose(file_); file_ = nullptr; }
}

std::vector<Record> WalReader::readAll() {
    std::vector<Record> records;

    while (true) {
        // --- read the 8-byte header: length(4) + crc(4) ---
        uint8_t header[8];
        size_t got = std::fread(header, 1, 8, file_);
        if (got == 0) break;          // clean end of file: we're done.
        if (got < 8)  break;          // torn header at the tail: stop here.

        uint32_t length    = getU32(header);
        uint32_t storedCrc = getU32(header + 4);

        // --- read the payload ---
        std::string payload;
        payload.resize(length);
        size_t pgot = std::fread(&payload[0], 1, length, file_);
        if (pgot < length) break;     // torn payload at the tail: stop here.

        // --- verify the checksum ---
        uint32_t actualCrc = crc32(payload.data(), payload.size());
        if (actualCrc != storedCrc) break;   // corrupted entry: stop here.

        // --- decode the record ---
        Record r;
        if (!deserializeRecord(payload, r)) break;  // malformed: stop here.

        records.push_back(std::move(r));
    }

    return records;
}

} // namespace quorumdb
