#pragma once
#include <array>
#include <map>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace util {

namespace splitter_sorter {

template <class score_tt>
static auto splitter(score_tt source) -> std::decay_t<score_tt> {
  return source;
}

/*
 * eg. 0-100 splitter: / 50
 *    elements: 33, 40, 66, 80, 100
 * splitter key: 0(0-49)   <33>, <40>
 * splitter key: 1(50-99)  <66>, <80>
 * splitter key: 2(100)    <100>
 *
 */

template <uint16_t level_vv, class score_tt, class element_key_tt,
          class element_tt>
class container {
  using score_type = std::decay_t<score_tt>;
  using element_key_type = std::decay_t<element_key_tt>;
  using element_type = std::decay_t<element_tt>;

  using elements = std::unordered_map<element_key_type, element_type>;
  using score_elements = std::map<score_type, elements>;

  using sort_container = std::map<score_type, score_elements>;
  using array_sort_container = std::array<score_elements, level_vv>;
  using element2score = std::unordered_map<element_key_type, score_type>;
  using element2iterator = std::unordered_map<
      element_key_type,
      std::tuple<std::size_t, typename score_elements::iterator,
                 typename elements::iterator>>;

  using rank_type = uint64_t;
  static constexpr rank_type not_exist_rank = 0;

 private:
  sort_container sc;
  element2score es;

  array_sort_container asc;
  element2iterator e2is;

 public:
  explicit container() = default;
  ~container() {
    // asc.clear();
    for (auto& one : asc) {
      one.clear();
    }
    e2is.clear();
  }

 public:
  [[maybe_unused]] bool insert(const score_type& k, const element_key_type& ek,
                               element_type&& e) {
    erase(ek);

    auto ks = splitter<score_type>(k);
    if (ks >= level_vv) {
      return false;
    }
    auto& asc_data = asc.at(ks);
    const auto ele = elements::value_type(ek, std::forward<element_type>(e));
    auto [se_it_fst, se_it_snd] = asc_data.emplace(k, elements{ele});
    if (!se_it_snd) {
      auto [e_it_fst, e_it_snd] = se_it_fst->second.emplace(ele);
      return e2is.emplace(ek, std::make_tuple(ks, se_it_fst, e_it_fst)).second;
    }
    return e2is
        .emplace(ek, std::make_tuple(ks, se_it_fst, se_it_fst->second.begin()))
        .second;
  }

  [[maybe_unused]] bool erase(const element_key_type& ek) {
    if (const auto& e2is_it = e2is.find(ek); e2is_it != e2is.end()) {
      auto [index, se_it, e_it] = e2is_it->second;

      se_it->second.erase(e_it);
      e2is.erase(e2is_it);

      return true;
    }
    return false;
  }

  [[nodiscard]] std::size_t size() const { return es.size(); }

  [[nodiscard]] rank_type rank(const element_key_type& ek) const {
    if (const auto& e2is_it = e2is.find(ek); e2is_it != e2is.end()) {
      auto [index, se_it, e_it] = e2is_it->second;

      rank_type result = 1;

      for (std::size_t i = 0; i <= index; ++i) {
        if (i == index) {
          for (auto each_se_it = asc.at(i).cbegin(); each_se_it != se_it;
               ++each_se_it) {
            result += each_se_it->second.size();
          }
          return result;
        }
        for (auto each_se_it = asc.at(i).cbegin();
             each_se_it != asc.at(i).cend(); ++each_se_it) {
          result += each_se_it->second.size();
        }
      }
      return result;
    }
    return not_exist_rank;

    if (const auto& ec_it = es.find(ek); ec_it != es.end()) {
      return inner_rank(ec_it->second);
    }
    return not_exist_rank;
  }

  [[nodiscard]] std::optional<score_type> score(
      const element_key_type& ek) const {
    if (const auto& ec_it = es.find(ek); ec_it != es.end()) {
      return ec_it->second;
    }
    return std::nullopt;
  }

  std::vector<element_key_type> range(rank_type l, rank_type r) const {
    std::vector<element_key_type> result{};

    if (r < l) {
      return result;
    }

    rank_type diff = r - l;

    auto begin = sc.cbegin();
    auto end = sc.cend();

    auto it = begin;

    while (it != end && it->second.size() < l) {
      ++it;
    }

    if (it == end)
      return result;

    rank_type count = 0;
    while (it != end) {
      for (const auto& it_ele_sc : it->second) {
        result.emplace_back(it_ele_sc.second.cbegin(), it_ele_sc.second.cend());
        if (++count >= diff) {
          return result;
        }
      }
      ++it;
    }
    return result;
  }

 private:
  rank_type inner_rank(const score_type& k) const {
    return inner_rank(splitter<score_type>(k), k);
  }

  rank_type inner_rank(const score_type& ks, const score_type& k) const {
    rank_type result = 1;
    if (const auto& sc_first_it = sc.find(ks); sc_first_it != sc.end()) {
      for (auto it = sc.cbegin(); it != sc_first_it; ++it) {
        for (auto sub_it = it->second.cbegin(); sub_it != it->second.cend();
             ++sub_it) {
          result += sub_it->second.size();
        }
      }

      if (const auto& sc_second_it = sc_first_it->second.find(k);
          sc_second_it != sc_first_it->second.end()) {
        for (auto sub_it = sc_first_it->second.cbegin(); sub_it != sc_second_it;
             ++sub_it) {
          result += sub_it->second.size();
        }
        return result;
      }
    }
    return not_exist_rank;
  }
};

template <uint16_t level_vv, class score_tt, class element_key_tt>
class container2 {
  using score_type = std::decay_t<score_tt>;
  using element_key_type = std::decay_t<element_key_tt>;

  using elements = std::list<element_key_type>;
  using score_elements = std::map<score_type, elements>;

  using sort_container = std::map<score_type, score_elements>;
  using array_sort_container = std::array<score_elements, level_vv>;
  using element2score = std::unordered_map<element_key_type, score_type>;
  using element2iterator = std::unordered_map<
      element_key_type,
      std::tuple<std::size_t, typename score_elements::iterator,
                 typename elements::iterator>>;

  using rank_type = uint64_t;
  static constexpr rank_type not_exist_rank = 0;

 private:
  sort_container sc;
  element2score es;

  array_sort_container asc;
  element2iterator e2is;

 public:
  explicit container2() = default;
  ~container2() {
    // asc.clear();
    for (auto& one : asc) {
      one.clear();
    }
    e2is.clear();
  }

 public:
  [[maybe_unused]] bool insert(const score_type& k,
                               const element_key_type& ek) {
    erase(ek);

    auto ks = splitter<score_type>(k);
    if (ks >= level_vv) {
      return false;
    }
    auto& asc_data = asc.at(ks);
    auto [se_it_fst, se_it_snd] = asc_data.emplace(k, elements{ek});
    if (!se_it_snd) {
      se_it_fst->second.emplace_front(ek);
    }
    return e2is
        .emplace(ek, std::make_tuple(ks, se_it_fst, se_it_fst->second.begin()))
        .second;
  }

  [[maybe_unused]] bool erase(const element_key_type& ek) {
    if (const auto& e2is_it = e2is.find(ek); e2is_it != e2is.end()) {
      auto [index, se_it, e_it] = e2is_it->second;

      se_it->second.erase(e_it);
      e2is.erase(e2is_it);

      return true;
    }
    return false;
  }

  [[nodiscard]] std::size_t size() const { return es.size(); }

  [[nodiscard]] rank_type rank(const element_key_type& ek) const {
    if (const auto& e2is_it = e2is.find(ek); e2is_it != e2is.end()) {
      auto [index, se_it, e_it] = e2is_it->second;

      rank_type result = 1;

      for (std::size_t i = 0; i <= index; ++i) {
        if (i == index) {
          for (auto each_se_it = asc.at(i).cbegin(); each_se_it != se_it;
               ++each_se_it) {
            result += each_se_it->second.size();
          }
          return result;
        }
        for (auto each_se_it = asc.at(i).cbegin();
             each_se_it != asc.at(i).cend(); ++each_se_it) {
          result += each_se_it->second.size();
        }
      }
      return result;
    }
    return not_exist_rank;
  }

  [[nodiscard]] std::optional<score_type> score(
      const element_key_type& ek) const {
    if (const auto& ec_it = es.find(ek); ec_it != es.end()) {
      return ec_it->second;
    }
    return std::nullopt;
  }

  std::vector<element_key_type> range(rank_type l, rank_type r) const {
    std::vector<element_key_type> result{};

    if (r < l) {
      return result;
    }

    rank_type diff = r - l;

    auto begin = sc.cbegin();
    auto end = sc.cend();

    auto it = begin;

    while (it != end && it->second.size() < l) {
      ++it;
    }

    if (it == end)
      return result;

    rank_type count = 0;
    while (it != end) {
      for (const auto& it_ele_sc : it->second) {
        result.emplace_back(it_ele_sc.second.cbegin(), it_ele_sc.second.cend());
        if (++count >= diff) {
          return result;
        }
      }
      ++it;
    }
    return result;
  }

 private:
  rank_type inner_rank(const score_type& k) const {
    return inner_rank(splitter<score_type>(k), k);
  }

  rank_type inner_rank(const score_type& ks, const score_type& k) const {
    rank_type result = 1;
    if (const auto& sc_first_it = sc.find(ks); sc_first_it != sc.end()) {
      for (auto it = sc.cbegin(); it != sc_first_it; ++it) {
        for (auto sub_it = it->second.cbegin(); sub_it != it->second.cend();
             ++sub_it) {
          result += sub_it->second.size();
        }
      }

      if (const auto& sc_second_it = sc_first_it->second.find(k);
          sc_second_it != sc_first_it->second.end()) {
        for (auto sub_it = sc_first_it->second.cbegin(); sub_it != sc_second_it;
             ++sub_it) {
          result += sub_it->second.size();
        }
        return result;
      }
    }
    return not_exist_rank;
  }
};
}  // namespace splitter_sorter

}  // namespace util


/* benchmark code


static void add_map_sorter(benchmark::State &state) {
  std::map<uint32_t, std::unordered_map<uint64_t, int>> xx;
  uint32_t seed = lcg_seed(12345);
  for (auto _ : state) {
    uint32_t score = lcg_rand(seed) % 1000;
    auto uuid = util::uuid_snowflake::generator::inst().nextid();
    // xx.emplace(score, std::unordered_map<uint64_t,
    // int>{}).first->second.emplace(uuid, 0);
    xx[score].emplace(uuid, 0);
  }
}

static void add_map_sorter2(benchmark::State &state) {
  std::map<uint32_t, std::unordered_map<uint64_t, int>> xx;
  uint32_t seed = lcg_seed(12345);
  for (auto _ : state) {
    uint32_t score = lcg_rand(seed) % 1000;
    auto uuid = util::uuid_snowflake::generator::inst().nextid();
    xx.emplace(score, std::unordered_map<uint64_t, int>{})
        .first->second.emplace(uuid, 0);
  }
}

static void add_map_sorter3(benchmark::State &state) {
  std::map<uint32_t, std::unordered_map<uint64_t, int>> xx;
  uint32_t seed = lcg_seed(12345);
  for (auto _ : state) {
    uint32_t score = lcg_rand(seed) % 1000;
    auto uuid = util::uuid_snowflake::generator::inst().nextid();
    auto [fst, snd] =
        xx.emplace(score, std::unordered_map<uint64_t, int>{{uuid, 0}});
    if (!snd)
      fst->second.emplace(uuid, 0);
  }
}

static void add_map_sorter4(benchmark::State &state) {
  std::map<uint32_t, std::unordered_map<uint64_t, int>> xx;
  uint32_t seed = lcg_seed(12345);
  for (auto _ : state) {
    uint32_t score = lcg_rand(seed) % 1000;
    auto uuid = util::uuid_snowflake::generator::inst().nextid();
    auto [fst, snd] = xx.insert_or_assign(
        score, std::unordered_map<uint64_t, int>{{uuid, 0}});
  }
}

namespace util::splitter_sorter {

template <>
static auto splitter<uint32_t>(uint32_t source) -> std::decay_t<uint32_t> {
  return source / 10;
}

}  // namespace util::splitter_sorter


static void add_splitter_sorter(benchmark::State &state) {
  static util::splitter_sorter::container<100, uint32_t, uint64_t, int> ssc;
  uint32_t seed = lcg_seed(12345);
  for (auto _ : state) {
    uint32_t score = lcg_rand(seed) % 1000;
    auto uuid = util::uuid_snowflake::generator::inst().nextid();
    ssc.insert(score, uuid, 0);
  }
}

static std::vector<uint64_t> add_splitter_sorter(
    util::splitter_sorter::container<100, uint32_t, uint64_t, int> &ssc, int count) {
  std::vector<uint64_t> result;
  result.reserve(count);
  uint32_t seed = lcg_seed(12345);
  for (auto i = 0; i < count; ++i) {
    uint32_t score = lcg_rand(seed) % 1000;
    auto uuid = util::uuid_snowflake::generator::inst().nextid();
    ssc.insert(score, uuid, 0);
    result.emplace_back(uuid);
  }
  return result;
}

static void rank_splitter_sorter(benchmark::State &state) {
  static util::splitter_sorter::container<100, uint32_t, uint64_t, int> ssc;

  uint32_t seed = lcg_seed(12345);
  int count = 1000000 / 10;
  auto uuids = add_splitter_sorter(ssc, count);

  for (auto _ : state) {
    if (uuids.empty()) break;
    auto uid = uuids.back();
    uuids.pop_back();
    auto __ = ssc.rank(uid);
  }
}


static void add_splitter_sorter2(benchmark::State &state) {
  static util::splitter_sorter::container2<100, uint32_t, uint64_t> ssc;
  uint32_t seed = lcg_seed(12345);
  for (auto _ : state) {
    uint32_t score = lcg_rand(seed) % 1000;
    auto uuid = util::uuid_snowflake::generator::inst().nextid();
    ssc.insert(score, uuid);
  }
}

static std::vector<uint64_t> add_splitter_sorter2(
    util::splitter_sorter::container2<100, uint32_t, uint64_t> &ssc,
    int count) {
  std::vector<uint64_t> result;
  result.reserve(count);
  uint32_t seed = lcg_seed(12345);
  for (auto i = 0; i < count; ++i) {
    uint32_t score = lcg_rand(seed) % 1000;
    auto uuid = util::uuid_snowflake::generator::inst().nextid();
    ssc.insert(score, uuid);
    result.emplace_back(uuid);
  }
  return result;
}

static void rank_splitter_sorter2(benchmark::State &state) {
  static util::splitter_sorter::container2<100, uint32_t, uint64_t> ssc;

  uint32_t seed = lcg_seed(12345);
  int count = 1000000 / 10;
  auto uuids = add_splitter_sorter2(ssc, count);

  for (auto _ : state) {
    if (uuids.empty())
      break;
    auto uid = uuids.back();
    uuids.pop_back();
    auto __ = ssc.rank(uid);
  }
}


BENCHMARK(add_map_sorter);
BENCHMARK(add_map_sorter2);
BENCHMARK(add_map_sorter3);
BENCHMARK(add_map_sorter4);
BENCHMARK(add_splitter_sorter);
BENCHMARK(rank_splitter_sorter);
BENCHMARK(add_splitter_sorter2);
BENCHMARK(rank_splitter_sorter2);

*/
