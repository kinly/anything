#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>

/// dep topological sort
///      A
///     / \
///    B   C
///    |  / \
///     D    E
/// -> A, B, C, D, E
namespace easy::utils {

template <class tt> class dep_sort {
public:
  struct relation;
  using relation_type = relation;

  struct result;
  using result_type = result;

  using value_type = tt;
  using map_type = std::unordered_map<value_type, relation_type>;
public:
  struct relation {
    std::size_t dependencies;
    std::unordered_set<value_type> dependents;
  };

  struct result {
    std::vector<value_type> sorted;
    std::vector<value_type> non_sorted;

    bool has_cycles() const { return non_sorted.empty() == false; }
  };

private:
  map_type _values;

public:
  bool has_node(const value_type& node) {
    return _values.contains(node);
  }

  void clear() { _values.clear(); }

  bool add_node(const value_type &node) {
    auto res = _values.insert({node, {}});
    return res.second;
  }

  bool add_node(value_type &&node) {
    auto res = _values.insert({std::move(node), {}});
    return res.second;
  }

  bool has_dependency(const value_type &node, const value_type &dependency) {
    if (!has_node(node))
      return false;
    const auto &value = _values.at(node);
    return value.dependents.find(dependency) != value.dependents.end();
  }

  bool add_dependency(const value_type &node, value_type &&dependency) {
    if (node == dependency)
      return false;
    auto res_value = _values.insert({std::move(dependency), {}});
    auto &dependents = res_value.first->second.dependents;
    if (dependents.find(node) == dependents.end()) {
      dependents.insert(node);
      _values[node].dependencies += 1;
      return true;
    }
    return false;
  }

  bool add_dependency(const value_type &node, const value_type &dependency) {
    if (node == dependency)
      return false;
    auto res_value = _values.insert({dependency, {}});
    auto &dependents = res_value.first->second.dependents;
    if (dependents.find(node) == dependents.end()) {
      dependents.insert(node);
      _values[node].dependencies += 1;
      return true;
    }
    return false;
  }

  template <template <typename, typename...> class container_tt, typename... container_tt_params>
  bool add_dependencies(
      const value_type &node,
      const container_tt<value_type, container_tt_params...> &dependencies) {
    for (const auto &one : dependencies) {
      if (!add_dependency(node, one))
        return false;
    }
    return true;
  }

  result_type sort() const {
    auto copyed = *this;
    result_type res;
    for (const auto &[node, relations] : copyed._values) {
      if (relations.dependencies == 0) {
        res.sorted.push_back(node);
      }
    }
    for (std::size_t index = 0; index < res.sorted.size(); ++index) {
      for (const auto &one : copyed._values[res.sorted.at(index)].dependents) {
        if (--copyed._values[one].dependencies == 0) {
          res.sorted.push_back(one);
        }
      }
    }
    for (const auto &[node, relations] : copyed._values) {
      if (relations.dependencies != 0) {
        res.non_sorted.push_back(node);
      }
    }
    return res;
  }
};
};
