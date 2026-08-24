#include "common/record.h"

#include <gtest/gtest.h>

namespace quorumdb {

TEST(RecordTest, NewerTimestampWins) {
    const Record older{"user:1", "old", 10, true};
    const Record newer{"user:1", "new", 11, false};

    EXPECT_TRUE(wins_over(newer, older));
    EXPECT_FALSE(wins_over(older, newer));
}

TEST(RecordTest, TombstoneWinsWhenTimestampsMatch) {
    const Record value{"user:1", "Naveen", 10, false};
    const Record tombstone{"user:1", "", 10, true};

    EXPECT_TRUE(wins_over(tombstone, value));
    EXPECT_FALSE(wins_over(value, tombstone));
}

TEST(RecordTest, LargerValueBreaksFinalTie) {
    const Record first{"user:1", "Naveen", 10, false};
    const Record second{"user:1", "Yashila", 10, false};

    EXPECT_TRUE(wins_over(second, first));
    EXPECT_FALSE(wins_over(first, second));
}

}  // namespace quorumdb
