#pragma once
#include <algorithm>
#include <functional>
#include <string>

namespace easy {
namespace utils {

constexpr inline bool is_ascii(char c) {
  return static_cast<unsigned char>(c) < 0x80;
}

constexpr inline bool is_lower_alpha(char c) { return (c >= 'a' && c <= 'z'); }

constexpr inline bool is_upper_alpha(char c) { return (c >= 'A' && c <= 'Z'); }

constexpr inline bool is_alpha(char c) {
  return is_lower_alpha(c) || is_upper_alpha(c);
}

constexpr inline bool is_numeric(char c) { return (c >= '0' && c <= '9'); }

constexpr inline bool is_hex_numeric(char c) {
  return is_numeric(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

constexpr inline bool is_white_space(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' ||
         c == '\v';
}

constexpr inline char to_lower(char c) { return is_alpha(c) ? (c | 0x20) : c; }

constexpr inline char to_upper(char c) {
  return is_alpha(c) ? (c & (~0x20)) : c;
}

constexpr inline char exchange_case(char c) {
  return is_alpha(c) ? (c ^ 0x20) : c;
}

inline bool start_with(const std::string &src, const char *pre,
                       std::size_t pre_size) {
  return src.size() >= pre_size && 0 == memcmp(src.data(), pre, pre_size);
}

inline bool end_with(const std::string &src, const char *suf,
                     std::size_t suf_size) {
  return src.size() >= suf_size &&
         0 == memcmp(src.data() + src.size() - suf_size, suf, suf_size);
}

inline bool start_with(const std::string &src, const char *pre) {
  return start_with(src, pre, strlen(pre));
}

inline bool end_with(const std::string &src, const char *suf) {
  return start_with(src, suf, strlen(suf));
}

inline bool start_with(const std::string &src, const std::string &pre) {
  return start_with(src, pre.data(), pre.size());
}

inline bool end_with(const std::string &src, const std::string &suf) {
  return end_with(src, suf.data(), suf.size());
}

template <class string_tt> string_tt trim_left(const string_tt &str) {
  typename string_tt::const_iterator it = str.begin();
  typename string_tt::const_iterator end = str.end();

  while (it != end && is_white_space(*it))
    ++it;
  return string_tt(it, end);
}

template <class string_tt> string_tt &trim_left_in_place(string_tt &str) {
  typename string_tt::iterator it = str.begin();
  typename string_tt::iterator end = str.end();

  while (it != end && is_white_space(*it))
    ++it;
  str.erase(str.begin(), it);
  return str;
}

template <class string_tt> string_tt trim_right(const string_tt &str) {
  // must use signed type
  long long pos = static_cast<long long>(str.size()) - 1;

  while (pos >= 0 && is_white_space(str[pos]))
    --pos;
  return string_tt(str, 0, pos + 1);
}

template <class string_tt> string_tt &trim_right_in_place(string_tt &str) {
  // must use signed type
  long long pos = static_cast<long long>(str.size()) - 1;

  while (pos >= 0 && is_white_space(str[pos]))
    --pos;
  str.resize(pos + 1);
  return str;
}

template <class string_tt> string_tt trim(const string_tt &str) {
  long long first = 0;
  long long last = static_cast<long long>(str.size()) - 1;

  while (first <= last && is_white_space(str[first]))
    ++first;
  while (last >= first && is_white_space(str[last]))
    --last;

  return string_tt(str, first, last - first + 1);
}

template <class string_tt> string_tt &trimInPlace(string_tt &str) {
  long long first = 0;
  long long last = static_cast<long long>(str.size()) - 1;

  while (first <= last && is_white_space(str[first]))
    ++first;
  while (last >= first && is_white_space(str[last]))
    --last;

  str.resize(last + 1);
  str.erase(0, first);

  return str;
}

template <class string_tt> string_tt to_lower(const string_tt &str) {
  typename string_tt::const_iterator it = str.begin();
  typename string_tt::const_iterator end = str.end();

  string_tt result;
  result.reserve(str.size());
  while (it != end)
    result += static_cast<typename string_tt::value_type>(to_lower(*it++));
  return result;
}

template <class string_tt> string_tt &to_lower_in_place(string_tt &str) {
  typename string_tt::iterator it = str.begin();
  typename string_tt::iterator end = str.end();

  while (it != end) {
    *it = static_cast<typename string_tt::value_type>(to_lower(*it));
    ++it;
  }
  return str;
}

template <class string_tt> string_tt to_upper(const string_tt &str) {
  typename string_tt::const_iterator it = str.begin();
  typename string_tt::const_iterator end = str.end();

  string_tt result;
  result.reserve(str.size());
  while (it != end)
    result += static_cast<typename string_tt::value_type>(to_upper(*it++));
  return result;
}

template <class string_tt> string_tt &to_upper_in_place(string_tt &str) {
  typename string_tt::iterator it = str.begin();
  typename string_tt::iterator end = str.end();

  while (it != end) {
    *it = static_cast<typename string_tt::value_type>(to_upper(*it));
    ++it;
  }
  return str;
}

template <class string_tt> string_tt exchange_case(const string_tt &str) {
  typename string_tt::const_iterator it = str.begin();
  typename string_tt::const_iterator end = str.end();

  string_tt result;
  result.reserve(str.size());
  while (it != end)
    result += static_cast<typename string_tt::value_type>(exchange_case(*it++));
  return result;
}

template <class string_tt> string_tt &exchange_case_in_place(string_tt &str) {
  typename string_tt::iterator it = str.begin();
  typename string_tt::iterator end = str.end();

  while (it != end) {
    *it = static_cast<typename string_tt::value_type>(exchange_case(*it));
    ++it;
  }
  return str;
}

template <class string_tt>
string_tt &replace_in_place(string_tt &str, const string_tt &from,
                            const string_tt &to,
                            typename string_tt::size_type start = 0) {
  if (from.empty() || from == to) {
    return str;
  }

  string_tt result;
  typename string_tt::size_type pos = 0;
  result.append(str, 0, start);
  do {
    pos = str.find(from, start);
    if (pos != string_tt::npos) {
      result.append(str, start, pos - start);
      result.append(to);
      start = pos + from.length();
    } else {
      result.append(str, start, str.size() - start);
    }
  } while (pos != string_tt::npos);
  str.swap(result);
  return str;
}

template <class string_tt>
string_tt &replace_in_place_ic(string_tt &str, const string_tt &from,
                               const string_tt &to,
                               typename string_tt::size_type start = 0) {
  if (from.empty() || from == to
      /*|| to_lower(from) == to_lower(to)*/) {
    return str;
  }

  typename string_tt::iterator beg = str.begin() + start;
  typename string_tt::iterator end = str.end();

  if (beg == end)
    return str;

  string_tt result;
  result.append(str, 0, start);

  // note! benchmark: > 10kb boyer_moore_searcher is better
  // auto bmsic = std::boyer_moore_searcher(from.begin(), from.end(),
  //     [](typename string_tt::value_type c) { return std::hash<typename
  //     string_tt::value_type>{}(std::toupper(c)); },
  //     [](typename string_tt::value_type c_l, typename string_tt::value_type
  //     c_r) { return std::toupper(c_l) == std::toupper(c_r); }
  // );
  // auto bms = std::boyer_moore_searcher(from.begin(), from.end());

  do {
    // typename string_tt::iterator it = std::search(beg, end, bmsic);
    // typename string_tt::iterator it = std::search(beg, end, bms);
    // typename string_tt::iterator it = std::search(beg, end, from.begin(),
    // from.end());
    typename string_tt::iterator it =
        std::search(beg, end, from.begin(), from.end(),
                    [](typename string_tt::value_type c_l,
                       typename string_tt::value_type c_r) {
                      return to_upper(c_l) == to_upper(c_r);
                    });
    if (it != str.end()) {
      result.append(str, start, it - beg);
      result.append(to);
      beg = (str.end() - it) > from.length() ? it + from.length() : str.end();
      start = beg - str.begin();
    } else {
      result.append(str, beg - str.begin(), str.end() - beg);
      beg = end;
    }
  } while (beg != end);
  str.swap(result);
  return str;
}

template <class string_tt>
string_tt &replace_in_place(string_tt &str,
                            const typename string_tt::value_type *from,
                            const typename string_tt::value_type *to,
                            typename string_tt::size_type start = 0) {
  if (from == nullptr)
    return str;

  string_tt result;
  typename string_tt::size_type pos = 0;
  typename string_tt::size_type fromLen = std::strlen(from);
  result.append(str, 0, start);
  do {
    pos = str.find(from, start);
    if (pos != string_tt::npos) {
      result.append(str, start, pos - start);
      result.append(to);
      start = pos + fromLen;
    } else
      result.append(str, start, str.size() - start);
  } while (pos != string_tt::npos);
  str.swap(result);
  return str;
}

template <class string_tt>
string_tt &replace_in_place(string_tt &str,
                            const typename string_tt::value_type from,
                            const typename string_tt::value_type to = 0,
                            typename string_tt::size_type start = 0) {
  if (from == to)
    return str;

  typename string_tt::size_type pos = 0;
  do {
    pos = str.find(from, start);
    if (pos != string_tt::npos) {
      if (to)
        str[pos] = to;
      else
        str.erase(pos, 1);
    }
  } while (pos != string_tt::npos);

  return str;
}

template <class string_tt>
string_tt &remove_in_place(string_tt &str,
                           const typename string_tt::value_type ch,
                           typename string_tt::size_type start = 0) {
  return replace_in_place(str, ch, 0, start);
}

template <class string_tt>
string_tt replace(const string_tt &str, const string_tt &from,
                  const string_tt &to,
                  typename string_tt::size_type start = 0) {
  string_tt result(str);
  replace_in_place(result, from, to, start);
  return result;
}

template <class string_tt>
string_tt replace(const string_tt &str,
                  const typename string_tt::value_type *from,
                  const typename string_tt::value_type *to,
                  typename string_tt::size_type start = 0) {
  string_tt result(str);
  replace_in_place(result, from, to, start);
  return result;
}

template <class string_tt>
string_tt replace(const string_tt &str,
                  const typename string_tt::value_type from,
                  const typename string_tt::value_type to = 0,
                  typename string_tt::size_type start = 0) {
  string_tt result(str);
  replace_in_place(result, from, to, start);
  return result;
}

template <class string_tt>
string_tt remove(const string_tt &str, const typename string_tt::value_type ch,
                 typename string_tt::size_type start = 0) {
  string_tt result(str);
  replace_in_place(result, ch, 0, start);
  return result;
}
} // namespace utils
} // namespace easy
