#include "cluster/hash_ring.h"

#include <unordered_set>

namespace quorumdb {
namespace {

constexpr std::uint64_t kFnvOffset = 14695981039346656037ULL;
constexpr std::uint64_t kFnvPrime = 1099511628211ULL;

}  // namespace

HashRing::HashRing(std::size_t virtual_nodes_per_node)
    : virtual_nodes_per_node_(virtual_nodes_per_node) {}

bool HashRing::add_node(const std::string& node_id) {
    if (node_id.empty() || virtual_nodes_per_node_ == 0 || node_tokens_.contains(node_id)) {
        return false;
    }

    std::vector<std::uint64_t> tokens;
    tokens.reserve(virtual_nodes_per_node_);

    for (std::size_t index = 0; index < virtual_nodes_per_node_; ++index) {
        std::uint64_t token = hash(node_id + "#" + std::to_string(index));
        while (ring_.contains(token)) {
            ++token;
        }

        ring_.emplace(token, node_id);
        tokens.push_back(token);
    }

    node_tokens_.emplace(node_id, std::move(tokens));
    return true;
}

bool HashRing::remove_node(const std::string& node_id) {
    const auto node = node_tokens_.find(node_id);
    if (node == node_tokens_.end()) {
        return false;
    }

    for (const std::uint64_t token : node->second) {
        ring_.erase(token);
    }
    node_tokens_.erase(node);
    return true;
}

std::optional<std::string> HashRing::owner_for_key(std::string_view key) const {
    return owner_for_token(hash(key));
}

std::optional<std::string> HashRing::owner_for_token(std::uint64_t token) const {
    if (ring_.empty()) {
        return std::nullopt;
    }

    auto owner = ring_.lower_bound(token);
    if (owner == ring_.end()) {
        owner = ring_.begin();
    }
    return owner->second;
}

std::vector<std::string> HashRing::replicas_for_key(
    std::string_view key, std::size_t replication_factor) const {
    std::vector<std::string> replicas;
    if (ring_.empty() || replication_factor == 0) {
        return replicas;
    }

    auto token = ring_.lower_bound(hash(key));
    if (token == ring_.end()) {
        token = ring_.begin();
    }

    std::unordered_set<std::string> selected_nodes;
    std::size_t visited_tokens = 0;
    while (visited_tokens < ring_.size() && replicas.size() < replication_factor) {
        if (selected_nodes.insert(token->second).second) {
            replicas.push_back(token->second);
        }

        ++token;
        if (token == ring_.end()) {
            token = ring_.begin();
        }
        ++visited_tokens;
    }

    return replicas;
}

std::size_t HashRing::node_count() const {
    return node_tokens_.size();
}

std::size_t HashRing::token_count() const {
    return ring_.size();
}

std::uint64_t HashRing::hash(std::string_view value) {
    std::uint64_t result = kFnvOffset;
    for (const unsigned char byte : value) {
        result ^= byte;
        result *= kFnvPrime;
    }
    return result;
}

}  // namespace quorumdb
