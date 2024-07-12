#pragma once

#include <tuple>

namespace tuple_helper {

template <class type_tt>
void hash_impl(std::size_t& offset, const type_tt& key) {
  std::hash<type_tt> hasher;
  offset ^= hasher(key) + 0x9e3779b9 + (offset << 6) + (offset >> 2);
}
};  // namespace tuple_helper
namespace std {
template <class... args_tt>
struct hash<std::tuple<args_tt...>> {
  template <class tuple_tt,
            std::size_t index = std::tuple_size<tuple_tt>::value - 1>
  struct impl {
    static void apply(std::size_t& offset, const tuple_tt& tuple) {
      impl<tuple_tt, index - 1>::apply(offset, tuple);
      tuple_helper::hash_impl(offset, std::get<index>(tuple));
    }
  };

  template <class tuple_tt>
  struct impl<tuple_tt, 0> {
    static void apply(std::size_t& offset, const tuple_tt& tuple) {
      tuple_helper::hash_impl(offset, std::get<0>(tuple));
    }
  };

  std::size_t operator()(const std::tuple<args_tt...>& key) const {
    std::size_t offset = 0;
    impl<std::tuple<args_tt...>>::apply(offset, key);
    return offset;
  }
};
};  // end namespace std

namespace tuple_helper {
struct printer {
  template <class ostream_tt, class tuple_tt,
            size_t index = std::tuple_size<tuple_tt>::value - 1>
  struct impl {
    static void apply(ostream_tt& ostr, const tuple_tt& tuple) {
      impl<ostream_tt, tuple_tt, index - 1>::apply(ostr, tuple);
      ostr << "," << std::get<index>(tuple);
    }
  };

  template <class ostream_tt, class tuple_tt>
  struct impl<ostream_tt, tuple_tt, 0> {
    static void apply(ostream_tt& ostr, const tuple_tt& tuple) {
      ostr << std::get<0>(tuple);
    }
  };
};
};  // namespace tuple_helper

template <class ostream_tt, class... args_tt>
ostream_tt& operator<<(ostream_tt& ostr,
                       const std::tuple<args_tt...>& tuple_value) {
  ostr << "(";
  tuple_helper::printer::impl<ostream_tt, std::tuple<args_tt...>>::apply(
      ostr, tuple_value);
  ostr << ")";

  return ostr;
}
