#include "cluster/hash_ring.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>
#include <unordered_set>

namespace quorumdb {

TEST(HashRingTest, EmptyRingHasNoOwner) {
    const HashRing ring;

    EXPECT_FALSE(ring.owner_for_key("key").has_value());
    EXPECT_TRUE(ring.replicas_for_key("key", 3).empty());
}

TEST(HashRingTest, AddsVirtualNodesAndRoutesDeterministically) {
    HashRing ring(4);
    HashRing matching_ring(4);
    ASSERT_TRUE(ring.add_node("node-a"));
    ASSERT_TRUE(ring.add_node("node-b"));
    ASSERT_TRUE(matching_ring.add_node("node-a"));
    ASSERT_TRUE(matching_ring.add_node("node-b"));

    EXPECT_EQ(ring.node_count(), 2);
    EXPECT_EQ(ring.token_count(), 8);
    EXPECT_EQ(ring.owner_for_key("user:1"), matching_ring.owner_for_key("user:1"));
    EXPECT_FALSE(ring.add_node("node-a"));
}

TEST(HashRingTest, WrapsAroundToFirstToken) {
    HashRing ring(4);
    ASSERT_TRUE(ring.add_node("node-a"));
    ASSERT_TRUE(ring.add_node("node-b"));

    EXPECT_EQ(ring.owner_for_token(std::numeric_limits<std::uint64_t>::max()),
              ring.owner_for_token(0));
}

TEST(HashRingTest, ReturnsDistinctReplicaNodes) {
    HashRing ring(8);
    ASSERT_TRUE(ring.add_node("node-a"));
    ASSERT_TRUE(ring.add_node("node-b"));
    ASSERT_TRUE(ring.add_node("node-c"));

    const auto replicas = ring.replicas_for_key("user:1", 3);
    const std::unordered_set<std::string> unique(replicas.begin(), replicas.end());

    EXPECT_EQ(replicas.size(), 3);
    EXPECT_EQ(unique.size(), 3);
}

TEST(HashRingTest, RemovesNodeFromRouting) {
    HashRing ring(4);
    ASSERT_TRUE(ring.add_node("node-a"));
    ASSERT_TRUE(ring.add_node("node-b"));
    ASSERT_TRUE(ring.remove_node("node-a"));

    EXPECT_EQ(ring.node_count(), 1);
    EXPECT_EQ(ring.token_count(), 4);
    EXPECT_EQ(ring.owner_for_key("any-key"), "node-b");
    EXPECT_FALSE(ring.remove_node("node-a"));
}

}  // namespace quorumdb
