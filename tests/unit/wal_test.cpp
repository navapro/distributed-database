// ============================================================================
//  wal_test.cpp  —  GoogleTest suite for the WAL.
//  OWNER: Yashila.
//
//  Same five behaviors as before, now written in the project's GoogleTest
//  style to match tests/unit/record_test.cpp. Each TEST proves ONE promise.
//
//  NOTE: Naveen's Record has no operator==, and that's his contract to own,
//  so we compare records field-by-field with a small local helper instead of
//  adding == to his struct.
// ============================================================================
#include "common/record.h"
#include "storage/wal/wal.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

namespace quorumdb {

// --- local test helpers ------------------------------------------------------

// Field-by-field equality, used only in tests.
static bool recordsEqual(const Record& a, const Record& b) {
    return a.key == b.key
        && a.value == b.value
        && a.timestamp == b.timestamp
        && a.tombstone == b.tombstone;
}

// Fresh temp path with any stale file removed first.
static std::string freshPath(const char* name) {
    std::string path = std::string("/tmp/quorumdb_") + name + ".wal";
    std::remove(path.c_str());
    return path;
}

static Record makeRecord(const std::string& k, const std::string& v,
                         std::uint64_t ts, bool tombstone = false) {
    return Record{k, v, ts, tombstone};
}

// --- Test 1: what we write is exactly what we read back. ---------------------
TEST(WalTest, RoundTripPreservesRecords) {
    const std::string path = freshPath("roundtrip");

    const std::vector<Record> in = {
        makeRecord("alpha", "one",   100),
        makeRecord("beta",  "two",   200),
        makeRecord("gamma", "three", 300),
    };

    {
        WalWriter w(path);
        for (const auto& r : in) w.append(r);
    }  // writer closes here

    WalReader reader(path);
    const std::vector<Record> out = reader.readAll();

    ASSERT_EQ(out.size(), in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        EXPECT_TRUE(recordsEqual(out[i], in[i]));
    }
}

// --- Test 2: an empty log reads back as zero records (not an error). ---------
TEST(WalTest, EmptyLogYieldsNoRecords) {
    const std::string path = freshPath("empty");

    { WalWriter w(path); }  // create the file, write nothing

    WalReader reader(path);
    EXPECT_TRUE(reader.readAll().empty());
}

// --- Test 3: tombstones (deletes) survive the round trip. --------------------
TEST(WalTest, TombstoneSurvivesRoundTrip) {
    const std::string path = freshPath("tombstone");

    {
        WalWriter w(path);
        w.append(makeRecord("k", "v", 10));                 // a normal write
        w.append(makeRecord("k", "",  20, /*tombstone=*/true)); // a delete
    }

    WalReader reader(path);
    const std::vector<Record> out = reader.readAll();

    ASSERT_EQ(out.size(), 2u);
    EXPECT_TRUE(out[1].tombstone);
    EXPECT_TRUE(out[1].value.empty());
}

// --- Test 4: durability — records survive a simulated crash/restart. --------
// We "crash" by destroying the writer, then open a brand-new reader on the
// same path, exactly like a process restart would.
TEST(WalTest, RecordsSurviveRestart) {
    const std::string path = freshPath("restart");

    {
        WalWriter w(path);
        w.append(makeRecord("persist", "yes", 1));
    }  // "crash" here — writer gone, but bytes are on disk

    WalReader reader(path);
    const std::vector<Record> out = reader.readAll();

    ASSERT_EQ(out.size(), 1u);
    EXPECT_TRUE(recordsEqual(out[0], makeRecord("persist", "yes", 1)));
}

// --- Test 5: a torn tail entry is dropped; everything before it is kept. -----
// Write 3 good records, then manually append junk that claims to be a header
// with no payload behind it, simulating a half-written entry from a crash.
TEST(WalTest, TornTailIsDroppedGoodRecordsKept) {
    const std::string path = freshPath("torn");

    {
        WalWriter w(path);
        w.append(makeRecord("a", "1", 1));
        w.append(makeRecord("b", "2", 2));
        w.append(makeRecord("c", "3", 3));
    }

    {
        std::ofstream f(path, std::ios::binary | std::ios::app);
        const char junk[4] = {0x10, 0x00, 0x00, 0x00};  // "payload is 16 bytes"...
        f.write(junk, 4);                                 // ...but nothing follows.
    }

    WalReader reader(path);
    EXPECT_EQ(reader.readAll().size(), 3u);
}

}  // namespace quorumdb
