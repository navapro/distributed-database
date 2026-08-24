#include "cluster/node_registry.h"

#include <gtest/gtest.h>

namespace quorumdb {

TEST(NodeRegistryTest, AddsAndFindsNode) {
    NodeRegistry registry;
    ASSERT_TRUE(registry.add({"node-a", "127.0.0.1", 7001, NodeStatus::Healthy}));

    const auto node = registry.get("node-a");
    ASSERT_TRUE(node.has_value());
    EXPECT_EQ(node->address, "127.0.0.1");
    EXPECT_EQ(node->port, 7001);
    EXPECT_EQ(registry.size(), 1);
}

TEST(NodeRegistryTest, RejectsInvalidAndDuplicateNodes) {
    NodeRegistry registry;

    EXPECT_FALSE(registry.add({"", "127.0.0.1", 7001, NodeStatus::Healthy}));
    EXPECT_TRUE(registry.add({"node-a", "127.0.0.1", 7001, NodeStatus::Healthy}));
    EXPECT_FALSE(registry.add({"node-a", "127.0.0.1", 7002, NodeStatus::Healthy}));
}

TEST(NodeRegistryTest, UpdatesNodeInformationAndStatus) {
    NodeRegistry registry;
    ASSERT_TRUE(registry.add({"node-a", "127.0.0.1", 7001, NodeStatus::Healthy}));

    ASSERT_TRUE(registry.update({"node-a", "10.0.0.1", 8001, NodeStatus::Suspected}));
    EXPECT_FALSE(registry.is_available("node-a"));

    ASSERT_TRUE(registry.update_status("node-a", NodeStatus::Healthy));
    EXPECT_TRUE(registry.is_available("node-a"));
    const auto updated = registry.get("node-a");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->address, "10.0.0.1");
}

TEST(NodeRegistryTest, RemovesNode) {
    NodeRegistry registry;
    ASSERT_TRUE(registry.add({"node-a", "127.0.0.1", 7001, NodeStatus::Healthy}));

    EXPECT_TRUE(registry.remove("node-a"));
    EXPECT_FALSE(registry.get("node-a").has_value());
    EXPECT_FALSE(registry.is_available("node-a"));
    EXPECT_FALSE(registry.remove("node-a"));
}

}  // namespace quorumdb
