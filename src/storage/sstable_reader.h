#pragma once

#include "common/record.h"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace quorumdb {

std::uint32_t sstable_checksum(const Record& record);

class SSTableReader {
public:
    explicit SSTableReader(std::filesystem::path path);

    bool read_all(std::vector<Record>& records) const;
    bool get(const std::string& key, std::optional<Record>& record) const;

private:
    std::filesystem::path path_;
};

}  // namespace quorumdb
