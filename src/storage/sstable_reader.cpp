#include "storage/sstable_reader.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <istream>
#include <utility>

namespace quorumdb {
namespace {

constexpr std::array<char, 4> kMagic{'Q', 'D', 'B', '1'};
constexpr std::uint32_t kMaxFieldSize = 16U * 1024U * 1024U;
constexpr std::uint32_t kFnvOffset = 2166136261U;
constexpr std::uint32_t kFnvPrime = 16777619U;

void add_checksum_byte(std::uint32_t& checksum, std::uint8_t byte) {
    checksum ^= byte;
    checksum *= kFnvPrime;
}

template <typename Integer>
void add_checksum_integer(std::uint32_t& checksum, Integer value) {
    for (std::size_t index = 0; index < sizeof(Integer); ++index) {
        add_checksum_byte(checksum, static_cast<std::uint8_t>(value & 0xffU));
        value >>= 8U;
    }
}

bool read_u32(std::istream& input, std::uint32_t& value) {
    std::array<unsigned char, 4> bytes{};
    if (!input.read(reinterpret_cast<char*>(bytes.data()), bytes.size())) {
        return false;
    }

    value = static_cast<std::uint32_t>(bytes[0]) |
            (static_cast<std::uint32_t>(bytes[1]) << 8U) |
            (static_cast<std::uint32_t>(bytes[2]) << 16U) |
            (static_cast<std::uint32_t>(bytes[3]) << 24U);
    return true;
}

bool read_u64(std::istream& input, std::uint64_t& value) {
    std::array<unsigned char, 8> bytes{};
    if (!input.read(reinterpret_cast<char*>(bytes.data()), bytes.size())) {
        return false;
    }

    value = 0;
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        value |= static_cast<std::uint64_t>(bytes[index]) << (index * 8U);
    }
    return true;
}

bool read_string(std::istream& input, std::uint32_t size, std::string& value) {
    value.resize(size);
    return static_cast<bool>(input.read(value.data(), size));
}

bool read_entry(std::istream& input, Record& record, bool& end_of_file) {
    if (input.peek() == std::char_traits<char>::eof()) {
        end_of_file = true;
        return true;
    }

    end_of_file = false;
    std::uint32_t key_size = 0;
    std::uint32_t value_size = 0;
    std::uint8_t tombstone = 0;
    std::uint32_t stored_checksum = 0;

    if (!read_u32(input, key_size) || !read_u32(input, value_size) ||
        !read_u64(input, record.timestamp) ||
        !input.read(reinterpret_cast<char*>(&tombstone), sizeof(tombstone))) {
        return false;
    }

    if (key_size > kMaxFieldSize || value_size > kMaxFieldSize || tombstone > 1) {
        return false;
    }

    record.tombstone = tombstone == 1;
    if (!read_string(input, key_size, record.key) ||
        !read_string(input, value_size, record.value) ||
        !read_u32(input, stored_checksum)) {
        return false;
    }

    return stored_checksum == sstable_checksum(record);
}

}  // namespace

std::uint32_t sstable_checksum(const Record& record) {
    std::uint32_t checksum = kFnvOffset;
    add_checksum_integer(checksum, static_cast<std::uint32_t>(record.key.size()));
    add_checksum_integer(checksum, static_cast<std::uint32_t>(record.value.size()));
    add_checksum_integer(checksum, record.timestamp);
    add_checksum_byte(checksum, record.tombstone ? 1U : 0U);

    for (const unsigned char byte : record.key) {
        add_checksum_byte(checksum, byte);
    }
    for (const unsigned char byte : record.value) {
        add_checksum_byte(checksum, byte);
    }

    return checksum;
}

SSTableReader::SSTableReader(std::filesystem::path path) : path_(std::move(path)) {}

bool SSTableReader::read_all(std::vector<Record>& records) const {
    records.clear();
    std::ifstream input(path_, std::ios::binary);
    if (!input) {
        return false;
    }

    std::array<char, 4> magic{};
    if (!input.read(magic.data(), magic.size()) || magic != kMagic) {
        return false;
    }

    while (true) {
        Record record;
        bool end_of_file = false;
        if (!read_entry(input, record, end_of_file)) {
            records.clear();
            return false;
        }
        if (end_of_file) {
            return true;
        }
        if (!records.empty() && records.back().key >= record.key) {
            records.clear();
            return false;
        }
        records.push_back(std::move(record));
    }
}

bool SSTableReader::get(const std::string& key, std::optional<Record>& record) const {
    record.reset();
    std::vector<Record> records;
    if (!read_all(records)) {
        return false;
    }

    const auto found = std::lower_bound(
        records.begin(), records.end(), key,
        [](const Record& candidate, const std::string& target) {
            return candidate.key < target;
        });

    if (found != records.end() && found->key == key) {
        record = *found;
    }
    return true;
}

}  // namespace quorumdb
