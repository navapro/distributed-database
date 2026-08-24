#include "storage/memtable.h"

#include <utility>

namespace quorumdb {

bool Memtable::put(Record record) {
    const auto existing = records_.find(record.key);

    if (existing == records_.end()) {
        size_bytes_ += record_size(record);
        records_.emplace(record.key, std::move(record));
        return true;
    }

    if (!wins_over(record, existing->second)) {
        return false;
    }

    size_bytes_ -= record_size(existing->second);
    size_bytes_ += record_size(record);
    existing->second = std::move(record);
    return true;
}

std::optional<Record> Memtable::get(const std::string& key) const {
    const auto record = records_.find(key);
    if (record == records_.end()) {
        return std::nullopt;
    }

    return record->second;
}

const Memtable::Records& Memtable::records() const {
    return records_;
}

std::size_t Memtable::size_bytes() const {
    return size_bytes_;
}

void Memtable::clear() {
    records_.clear();
    size_bytes_ = 0;
}

std::size_t Memtable::record_size(const Record& record) {
    return record.key.size() + record.value.size() + sizeof(record.timestamp) +
           sizeof(record.tombstone);
}

}  // namespace quorumdb
