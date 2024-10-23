#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace rank {

class container_interface {
 public:
  virtual ~container_interface() = default;
  // todo:
  // virtual std::string serialize() = 0;
  // virtual std::string unserialize() = 0;
};

template <std::size_t count_vv, class sort_key_tt, class element_tt,
          class element_key_tt, class compare_tt = std::less<sort_key_tt> >
class container : public container_interface {
  using sort_data =
      std::map<sort_key_tt, std::pair<element_key_tt, element_tt>, compare_tt>;
  using sort_data_iterator = typename sort_data::iterator;

 private:
  const std::size_t _count = count_vv;
  sort_data _data;
  std::unordered_map<element_key_tt, sort_data_iterator> _elements;
  std::unordered_set<element_key_tt> _dirty_elements;

 public:
  container() = default;

  container(compare_tt&& compare_call) : _data(compare_call) {}

  ~container() override = default;

  [[maybe_unused]] bool insert(const sort_key_tt& skey, const element_tt& ev, const element_key_tt& ekey) {
    remove(ekey);

    if (auto [data_it, res] = _data.emplace(skey, std::make_pair(ekey, ev));
        res) {
      if (auto [ele_it, res] = _elements.emplace(ekey, data_it); res) {
        _dirty_elements.emplace(ekey);
      }
    }

    while (_data.size() > _count) {
      auto last_data_it = std::prev(_data.end());
      remove(last_data_it->second.first);
    }

    return exist(ekey);
  }

  [[maybe_unused]] bool remove(const element_key_tt& ekey) {
    if (const auto it = _elements.find(ekey); it != _elements.end()) {
      _dirty_elements.emplace(ekey);

      _data.erase(it->second);
      _elements.erase(it);
      return true;
    }
    return false;
  }

  bool exist(const element_key_tt& ekey) const {
    return _elements.contains(ekey);
  }
};

}; // namespace rank

namespace rank_test {

struct sort_key {
  uint32_t level = 0;
  uint32_t exp = 0;

  uint64_t ts = 0;
  uint64_t auto_increment = 0;
};

using element_key = uint64_t;

struct element_value {
  std::string name;
};
};  // namespace rank_test

template <>
struct std::less<rank_test::sort_key> {
  bool operator()(const rank_test::sort_key& left,
                  const rank_test::sort_key& right) const {
    return left.level < right.level ||
           (left.level == right.level && left.exp < right.exp) ||
           (left.level == right.level && left.exp == right.exp &&
            left.ts < right.ts) ||
           (left.level == right.level && left.exp == right.exp &&
            left.ts == right.ts && left.auto_increment < right.auto_increment);
  }
};
template <>
struct std::greater<rank_test::sort_key> {
  bool operator()(const rank_test::sort_key& left,
                  const rank_test::sort_key& right) const {
    return left.level > right.level ||
           (left.level == right.level && left.exp > right.exp) ||
           (left.level == right.level && left.exp == right.exp &&
            left.ts > right.ts) ||
           (left.level == right.level && left.exp == right.exp &&
            left.ts == right.ts && left.auto_increment > right.auto_increment);
  }
};

rank::container<10, rank_test::sort_key, rank_test::element_value,
                rank_test::element_key>
    rc;

rank::container<10, rank_test::sort_key, rank_test::element_value,
                rank_test::element_key, std::greater<rank_test::sort_key>>
    rcg;

for (int i = 0; i < 100; ++i) {
  rank_test::sort_key sk{1, 1, 1, i};
  rank_test::element_value ev{"name_" + std::to_string(i)};

  rc.insert(sk, ev, i);
  rcg.insert(sk, ev, i);
}

namespace lua_rank_test {

struct sort_key {
  std::unordered_map<std::string, int64_t> keys;
};

};  // namespace lua_rank_test

std::vector<std::pair<std::string, bool>> compares{{"level", true},
                                                   {"exp", true}};
auto lua_comp = [compares](const lua_rank_test::sort_key& left,
                           const lua_rank_test::sort_key& right) {
  for (const auto& one : compares) {
    auto lv = left.keys.at(one.first);
    auto rv = right.keys.at(one.first);
    if (lv == rv) 
      continue;
    if (one.second)  // sort down
      return lv > rv;
    return lv < rv;
  }
  return false;
};

rank::container<10, rank_test::sort_key, rank_test::element_value,
                rank_test::element_key, decltype(lua_comp)>
    rcl(lua_comp);
