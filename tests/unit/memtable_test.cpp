#include "storage/memtable.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

namespace quorumdb {

TEST(MemtableTest, InsertsAndRetrievesRecord) {
    Memtable memtable;
    const Record record{"user:1", "Naveen", 10, false};

    EXPECT_TRUE(memtable.put(record));
    const auto stored = memtable.get("user:1");
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->value, "Naveen");
    EXPECT_FALSE(memtable.get("missing").has_value());
}

TEST(MemtableTest, KeepsRecordsSortedByKey) {
    Memtable memtable;
    memtable.put({"charlie", "3", 1, false});
    memtable.put({"alpha", "1", 1, false});
    memtable.put({"bravo", "2", 1, false});

    auto record = memtable.records().begin();
    EXPECT_EQ((record++)->first, "alpha");
    EXPECT_EQ((record++)->first, "bravo");
    EXPECT_EQ(record->first, "charlie");
}

TEST(MemtableTest, KeepsNewestRecord) {
    Memtable memtable;
    memtable.put({"user:1", "new", 20, false});

    EXPECT_FALSE(memtable.put({"user:1", "old", 10, false}));
    const auto stored = memtable.get("user:1");
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->value, "new");
}

TEST(MemtableTest, KeepsTombstoneWhenTimestampsMatch) {
    Memtable memtable;
    memtable.put({"user:1", "Naveen", 10, false});

    EXPECT_TRUE(memtable.put({"user:1", "", 10, true}));
    const auto stored = memtable.get("user:1");
    ASSERT_TRUE(stored.has_value());
    EXPECT_TRUE(stored->tombstone);
}

TEST(MemtableTest, TracksSizeAndClearsRecords) {
    Memtable memtable;
    memtable.put({"key", "a", 10, false});
    const std::size_t first_size = 3 + 1 + sizeof(std::uint64_t) + sizeof(bool);
    EXPECT_EQ(memtable.size_bytes(), first_size);

    memtable.put({"key", "longer", 20, false});
    const std::size_t replacement_size = 3 + 6 + sizeof(std::uint64_t) + sizeof(bool);
    EXPECT_EQ(memtable.size_bytes(), replacement_size);

    memtable.clear();
    EXPECT_TRUE(memtable.records().empty());
    EXPECT_EQ(memtable.size_bytes(), 0);
}

}  // namespace quorumdb
