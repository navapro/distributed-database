#pragma once

#include <cstdint>
#include <string>

namespace quorumdb {

struct Record {
    std::string key;
    std::string value;
    std::uint64_t timestamp;
    bool tombstone;
};

bool wins_over(const Record& candidate, const Record& current);

}  // namespace quorumdb
