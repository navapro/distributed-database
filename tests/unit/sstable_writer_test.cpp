// ============================================================================
//  sstable_writer_test.cpp  —  GoogleTest suite for the SSTable writer.
//  OWNER: Yashila.
//
//  The whole strategy here: prove the writer is correct by reading its output
//  back with NAVEEN'S REAL SSTableReader. If his reader accepts and correctly
//  decodes what we wrote, the two are provably format-compatible.
// ============================================================================
#include "common/record.h"
#include "storage/memtable.h"
#include "storage/sstable_reader.h"
#include "storage/sstable_writer.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace quorumdb {

// --- helpers ----------------------------------------------------------------

static bool recordsEqual(const Record& a, const Record& b) {
    return a.key == b.key && a.value == b.value &&
           a.timestamp == b.timestamp && a.tombstone == b.tombstone;
}

static std::filesystem::path freshPath(const char* name) {
    auto path = std::filesystem::temp_directory_path() /
                (std::string("quorumdb_") + name + ".sst");
    std::error_code ec;
    std::filesystem::remove(path, ec);
    return path;
}

static Record makeRecord(const std::string& k, const std::string& v,
                         std::uint64_t ts, bool tombstone = false) {
    return Record{k, v, ts, tombstone};
}

// --- Test 1: vector round-trips through Naveen's reader. ---------------------
TEST(SSTableWriterTest, VectorRoundTripsThroughReader) {
    const auto path = freshPath("vec_roundtrip");

    const std::vector<Record> in = {
        makeRecord("apple",  "red",    100),
        makeRecord("banana", "yellow", 200),
        makeRecord("cherry", "dark",   300),
    };

    ASSERT_TRUE(SSTableWriter::write(path, in));

    SSTableReader reader(path);
    std::vector<Record> out;
    ASSERT_TRUE(reader.read_all(out));           // his reader accepts our file

    ASSERT_EQ(out.size(), in.size());
    for (size_t i = 0; i < in.size(); ++i)
        EXPECT_TRUE(recordsEqual(out[i], in[i]));
}

// --- Test 2: a real Memtable flush round-trips through the reader. -----------
TEST(SSTableWriterTest, MemtableFlushRoundTrips) {
    const auto path = freshPath("memtable_flush");

    // Insert out of order on purpose; the map sorts them for us.
    Memtable mt;
    mt.put(makeRecord("gamma", "3", 30));
    mt.put(makeRecord("alpha", "1", 10));
    mt.put(makeRecord("beta",  "2", 20));

    ASSERT_TRUE(SSTableWriter::write(path, mt.records()));

    SSTableReader reader(path);
    std::vector<Record> out;
    ASSERT_TRUE(reader.read_all(out));

    ASSERT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0].key, "alpha");   // proves output is sorted
    EXPECT_EQ(out[1].key, "beta");
    EXPECT_EQ(out[2].key, "gamma");
}

// --- Test 3: tombstones survive the write and read back as deletes. ---------
TEST(SSTableWriterTest, TombstoneSurvives) {
    const auto path = freshPath("tombstone");

    const std::vector<Record> in = {
        makeRecord("keep",   "value", 10),
        makeRecord("remove", "",      20, /*tombstone=*/true),
    };

    ASSERT_TRUE(SSTableWriter::write(path, in));

    SSTableReader reader(path);
    std::optional<Record> got;
    ASSERT_TRUE(reader.get("remove", got));
    ASSERT_TRUE(got.has_value());
    EXPECT_TRUE(got->tombstone);
    EXPECT_TRUE(got->value.empty());
}

// --- Test 4: point lookups via the reader find and miss correctly. ----------
TEST(SSTableWriterTest, PointLookupHitsAndMisses) {
    const auto path = freshPath("point_lookup");

    const std::vector<Record> in = {
        makeRecord("k1", "v1", 1),
        makeRecord("k2", "v2", 2),
        makeRecord("k3", "v3", 3),
    };
    ASSERT_TRUE(SSTableWriter::write(path, in));

    SSTableReader reader(path);

    std::optional<Record> hit;
    ASSERT_TRUE(reader.get("k2", hit));
    ASSERT_TRUE(hit.has_value());
    EXPECT_EQ(hit->value, "v2");

    std::optional<Record> miss;
    ASSERT_TRUE(reader.get("nope", miss));   // lookup succeeds...
    EXPECT_FALSE(miss.has_value());          // ...but finds nothing
}

// --- Test 5: an empty table writes just the magic and reads as zero records. -
TEST(SSTableWriterTest, EmptyTableIsValid) {
    const auto path = freshPath("empty");

    const std::vector<Record> in;               // no records
    ASSERT_TRUE(SSTableWriter::write(path, in));

    SSTableReader reader(path);
    std::vector<Record> out;
    ASSERT_TRUE(reader.read_all(out));          // still a valid file
    EXPECT_TRUE(out.empty());
}

// --- Test 6: the writer REJECTS unsorted input instead of writing junk. -----
TEST(SSTableWriterTest, RejectsUnsortedInput) {
    const auto path = freshPath("unsorted");

    const std::vector<Record> bad = {
        makeRecord("b", "1", 1),
        makeRecord("a", "2", 2),   // out of order
    };
    EXPECT_FALSE(SSTableWriter::write(path, bad));
}

// --- Test 7: the writer REJECTS duplicate keys (reader requires uniqueness). -
TEST(SSTableWriterTest, RejectsDuplicateKeys) {
    const auto path = freshPath("dupes");

    const std::vector<Record> dupes = {
        makeRecord("same", "1", 1),
        makeRecord("same", "2", 2),   // duplicate key
    };
    EXPECT_FALSE(SSTableWriter::write(path, dupes));
}

}  // namespace quorumdb
