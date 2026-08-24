#include "cluster/node_registry.h"

#include <utility>

namespace quorumdb {

bool NodeRegistry::add(NodeInfo node) {
    if (!is_valid(node) || nodes_.contains(node.id)) {
        return false;
    }

    const std::string node_id = node.id;
    nodes_.emplace(node_id, std::move(node));
    return true;
}

bool NodeRegistry::update(NodeInfo node) {
    if (!is_valid(node)) {
        return false;
    }

    const auto existing = nodes_.find(node.id);
    if (existing == nodes_.end()) {
        return false;
    }

    existing->second = std::move(node);
    return true;
}

bool NodeRegistry::update_status(const std::string& node_id, NodeStatus status) {
    const auto node = nodes_.find(node_id);
    if (node == nodes_.end()) {
        return false;
    }

    node->second.status = status;
    return true;
}

bool NodeRegistry::remove(const std::string& node_id) {
    return nodes_.erase(node_id) == 1;
}

std::optional<NodeInfo> NodeRegistry::get(const std::string& node_id) const {
    const auto node = nodes_.find(node_id);
    if (node == nodes_.end()) {
        return std::nullopt;
    }

    return node->second;
}

bool NodeRegistry::is_available(const std::string& node_id) const {
    const auto node = nodes_.find(node_id);
    return node != nodes_.end() && node->second.status == NodeStatus::Healthy;
}

std::size_t NodeRegistry::size() const {
    return nodes_.size();
}

bool NodeRegistry::is_valid(const NodeInfo& node) {
    return !node.id.empty() && !node.address.empty() && node.port != 0;
}

}  // namespace quorumdb
