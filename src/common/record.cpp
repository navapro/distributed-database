#include "common/record.h"

namespace quorumdb {

bool wins_over(const Record& candidate, const Record& current) {
    if (candidate.timestamp != current.timestamp) {
        return candidate.timestamp > current.timestamp;
    }

    if (candidate.tombstone != current.tombstone) {
        return candidate.tombstone;
    }

    return candidate.value > current.value;
}

}  // namespace quorumdb
