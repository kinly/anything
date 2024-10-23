#pragma once
#include <functional>
#include <list>
#include <mutex>
#include <unordered_map>

namespace easy::lru {

template <class key_tt, class value_tt, size_t max_size>
class cache {
 public:
  using element_type = std::pair<key_tt, value_tt>;
  using array_type = std::list<element_type>;
  using map_type =
      std::unordered_map<key_tt, typename array_type::const_iterator>;

 public:
  // https://en.cppreference.com/w/cpp/types/void_t
  // slice object trait
  template <class, class = void>
  struct has_on_rem : std::false_type {};
  template <class slice_object>
  struct has_on_rem<slice_object,
                    std::void_t<decltype(std::declval<slice_object>().on_rem(
                        std::declval<key_tt &>(), std::declval<value_tt &>()))>>
      : std::true_type {};

  template <class slice_object>
  static constexpr bool has_on_rem_v = has_on_rem<slice_object>::value;

 private:
  array_type _data_array;
  map_type _data_map;
  mutable std::mutex _mut;

  std::function<void(key_tt &key, value_tt &value)> _on_rem;

 private:
  template <class slice_tt>
  void do_on_rem(slice_tt &s, key_tt &key, value_tt &value) {
    if constexpr (has_on_rem_v<slice_tt>) {
      s.on_rem(key, value);
    }
  }

 public:
  template <class... slice_args>
  explicit cache(slice_args &&...sargs) {
    // https://stackoverflow.com/questions/47496358/c-lambdas-how-to-capture-variadic-parameter-pack-from-the-upper-scope
    if constexpr (sizeof...(slice_args) > 0) {
      _on_rem = [this, ... sargs = std::forward<slice_args>(sargs)](
                    key_tt &key, value_tt &value) mutable {
        (do_on_rem(sargs, key, value), ...);
      };
    }
  }

  ~cache() {
    std::lock_guard<std::mutex> lock(_mut);
    _data_map.clear();
    _data_array.clear();
  }

  // non-copyable
  cache(const cache &) = delete;
  cache(cache &&) = delete;
  cache &operator=(const cache &) = delete;

  bool add(const key_tt &key, const value_tt &value) {
    std::lock_guard<std::mutex> lock(_mut);
    if (max_size == 0)
      return false;

    const auto &iter = _data_map.find(key);
    if (iter != _data_map.cend()) {
      _data_array.erase(iter->second);
      _data_map.erase(iter);
    }
    if (_data_array.size() >= max_size) {
      auto last = _data_array.back();
      _data_array.pop_back();
      _data_map.erase(last.first);

      if (_on_rem) {
        _on_rem(last.first, last.second);
      }
    }

    _data_array.emplace_front(key, value);
    _data_map.emplace(key, _data_array.begin());
    return true;
  }

  void rem(const key_tt &key) {
    std::lock_guard<std::mutex> lock(_mut);
    const auto &iter = _data_map.find(key);
    if (iter == _data_map.end())
      return;

    auto element = *(iter->second);

    _data_array.erase(iter->second);
    _data_map.erase(iter);

    if (_on_rem) {
      _on_rem(element.first, element.second);
    }
  }

  value_tt *get(const key_tt &key) {
    std::lock_guard<std::mutex> lock(_mut);

    const auto &iter = _data_map.find(key);
    if (iter == _data_map.end())
      return nullptr;

    auto element = *(iter->second);

    _data_array.erase(iter->second);
    _data_map.erase(iter);

    _data_array.emplace_front(element);
    _data_map.emplace(element.first, _data_array.begin());
    return element.second;
  }

  void pop() {
    std::lock_guard<std::mutex> lock(_mut);
    auto last = _data_array.back();
    _data_array.pop_back();
    _data_map.erase(last.first);

    if (_on_rem) {
      _on_rem(last.first, last.second);
    }
  }
};

};  // namespace easy::lru
