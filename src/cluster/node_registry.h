#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

namespace quorumdb {

enum class NodeStatus {
    Healthy,
    Suspected,
    Down,
};

struct NodeInfo {
    std::string id;
    std::string address;
    std::uint16_t port;
    NodeStatus status{NodeStatus::Healthy};
};

class NodeRegistry {
public:
    bool add(NodeInfo node);
    bool update(NodeInfo node);
    bool update_status(const std::string& node_id, NodeStatus status);
    bool remove(const std::string& node_id);

    std::optional<NodeInfo> get(const std::string& node_id) const;
    bool is_available(const std::string& node_id) const;
    std::size_t size() const;

private:
    static bool is_valid(const NodeInfo& node);

    std::unordered_map<std::string, NodeInfo> nodes_;
};

}  // namespace quorumdb
