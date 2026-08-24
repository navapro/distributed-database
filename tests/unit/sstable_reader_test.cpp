#include "storage/sstable_reader.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace quorumdb {
namespace {

void write_u32(std::ostream& output, std::uint32_t value) {
    for (int index = 0; index < 4; ++index) {
        output.put(static_cast<char>(value & 0xffU));
        value >>= 8U;
    }
}

void write_u64(std::ostream& output, std::uint64_t value) {
    for (int index = 0; index < 8; ++index) {
        output.put(static_cast<char>(value & 0xffU));
        value >>= 8U;
    }
}

bool write_sstable(const std::filesystem::path& path,
                   const std::vector<Record>& records,
                   bool corrupt_checksum = false) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        return false;
    }

    output.write("QDB1", 4);
    for (const Record& record : records) {
        write_u32(output, static_cast<std::uint32_t>(record.key.size()));
        write_u32(output, static_cast<std::uint32_t>(record.value.size()));
        write_u64(output, record.timestamp);
        output.put(record.tombstone ? 1 : 0);
        output.write(record.key.data(), record.key.size());
        output.write(record.value.data(), record.value.size());

        std::uint32_t checksum = sstable_checksum(record);
        if (corrupt_checksum) {
            checksum ^= 1U;
        }
        write_u32(output, checksum);
    }
    return static_cast<bool>(output);
}

class SSTableReaderTest : public testing::Test {
protected:
    void SetUp() override {
        const std::string test_name =
            testing::UnitTest::GetInstance()->current_test_info()->name();
        path_ = std::filesystem::temp_directory_path() / ("quorumdb_" + test_name + ".sst");
        std::filesystem::remove(path_);
    }

    void TearDown() override {
        std::filesystem::remove(path_);
    }

    std::filesystem::path path_;
};

TEST_F(SSTableReaderTest, ReadsRecordsInOrder) {
    const std::vector<Record> expected{
        {"alpha", "1", 10, false},
        {"bravo", "2", 11, false},
    };
    ASSERT_TRUE(write_sstable(path_, expected));

    std::vector<Record> records;
    ASSERT_TRUE(SSTableReader(path_).read_all(records));
    ASSERT_EQ(records.size(), 2);
    EXPECT_EQ(records[0].key, "alpha");
    EXPECT_EQ(records[1].key, "bravo");
}

TEST_F(SSTableReaderTest, FindsRecordAndReportsMissingKey) {
    ASSERT_TRUE(write_sstable(path_, {{"alpha", "1", 10, false}}));
    SSTableReader reader(path_);

    std::optional<Record> record;
    ASSERT_TRUE(reader.get("alpha", record));
    ASSERT_TRUE(record.has_value());
    EXPECT_EQ(record->value, "1");

    ASSERT_TRUE(reader.get("missing", record));
    EXPECT_FALSE(record.has_value());
}

TEST_F(SSTableReaderTest, ReturnsTombstone) {
    ASSERT_TRUE(write_sstable(path_, {{"deleted", "", 12, true}}));

    std::optional<Record> record;
    ASSERT_TRUE(SSTableReader(path_).get("deleted", record));
    ASSERT_TRUE(record.has_value());
    EXPECT_TRUE(record->tombstone);
}

TEST_F(SSTableReaderTest, RejectsBadChecksum) {
    ASSERT_TRUE(write_sstable(path_, {{"alpha", "1", 10, false}}, true));

    std::vector<Record> records;
    EXPECT_FALSE(SSTableReader(path_).read_all(records));
    EXPECT_TRUE(records.empty());
}

}  // namespace
}  // namespace quorumdb
