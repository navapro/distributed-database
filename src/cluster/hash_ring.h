#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace quorumdb {

class HashRing {
public:
    explicit HashRing(std::size_t virtual_nodes_per_node = 32);

    bool add_node(const std::string& node_id);
    bool remove_node(const std::string& node_id);

    std::optional<std::string> owner_for_key(std::string_view key) const;
    std::optional<std::string> owner_for_token(std::uint64_t token) const;
    std::vector<std::string> replicas_for_key(
        std::string_view key, std::size_t replication_factor) const;

    std::size_t node_count() const;
    std::size_t token_count() const;

private:
    static std::uint64_t hash(std::string_view value);

    std::size_t virtual_nodes_per_node_;
    std::map<std::uint64_t, std::string> ring_;
    std::unordered_map<std::string, std::vector<std::uint64_t>> node_tokens_;
};

}  // namespace quorumdb
