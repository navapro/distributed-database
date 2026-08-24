#pragma once

#include "common/record.h"

#include <cstddef>
#include <map>
#include <optional>
#include <string>

namespace quorumdb {

class Memtable {
public:
    using Records = std::map<std::string, Record>;

    bool put(Record record);
    std::optional<Record> get(const std::string& key) const;
    const Records& records() const;
    std::size_t size_bytes() const;
    void clear();

private:
    static std::size_t record_size(const Record& record);

    Records records_;
    std::size_t size_bytes_{0};
};

}  // namespace quorumdb
