#pragma once
#include <functional>
#include <set>
#include <unordered_map>

namespace easy::lfu {

template <class key_tt, class value_tt, size_t max_size>
class cache {
 public:
  struct node {
    key_tt key{};
    value_tt value{};

    size_t count = 0;
    size_t sequence = 0;

    bool operator==(const node& src) const { return key == src.key; }

    bool operator<(const node& src) const {
      return count == src.count ? sequence < src.sequence : count < src.count;
    }
  };

 public:
  using remove_callback = std::function<void(key_tt&& key, value_tt&& value)>;
  using node_container = std::set<node>;
  using node_map =
      std::unordered_map<key_tt, typename node_container::iterator>;

 private:
  node_container _nodes;
  node_map _key2node;

  remove_callback _on_remove;

  std::size_t _sequence = 0;

 public:
  explicit cache(remove_callback&& on_remove) : _on_remove(on_remove) {}

  // non-copyable
  cache(const cache&) = delete;
  cache(cache&&) = delete;
  cache& operator=(const cache&) = delete;

  bool put(const key_tt& key, const value_tt& value) {
    if (max_size == 0)
      return false;

    if (auto iter = _key2node.find(key); iter != _key2node.end()) {
      auto one = *(iter->second);
      one.count += 1;
      one.sequence = ++_sequence;
      _nodes.erase(iter->second);
      iter->second = _nodes.emplace(one).first;
      return true;
    }

    while (_nodes.size() >= max_size) {
      auto one = (*_nodes.begin());
      _key2node.erase(_nodes.begin()->key);
      _nodes.erase(_nodes.begin());

      if (_on_remove) {
        _on_remove(std::move(one.key), std::move(one.value));
      }
    }

    _key2node[key] = _nodes.emplace(key, value, 0, ++_sequence).first;
    return true;
  }

  value_tt* get(const key_tt& key) {
    if (auto iter = _key2node.find(key); iter != _key2node.end()) {
      auto ignore_res = put(key, iter->second->value);
      iter = _key2node.find(key);
      return const_cast<value_tt*>(&(iter->second->value));
    }
    return nullptr;
  }
};

};  // namespace easy::lfu
