#include <iostream>
#include <algorithm>

namespace inlay {

    template<typename F, std::size_t... S>
    constexpr void static_for(F&& function, std::index_sequence<S...>) {
        using unpack_t = int[];
        (void)unpack_t {
            (static_cast<void>(function(std::integral_constant<std::size_t, S>{})), 0)..., 0
        };
    }

    template<std::size_t iterations, typename F>
    constexpr void static_for(F&& function) {
        static_for(std::forward<F>(function), std::make_index_sequence<iterations>());
    }

    template<size_t index, typename T, typename... Ts,
        typename std::enable_if<index == 0>::type* = nullptr
    >
    inline constexpr decltype(auto) get(T&& t, Ts&&... ts) {
        return std::forward<T>(t);
    }

    template<size_t index, typename T, typename... Ts,
        typename std::enable_if<(index > 0 && index <= sizeof...(Ts))>::type* = nullptr
    >
    inline constexpr decltype(auto) get(T&& t, Ts&&... ts) {
        return get<index - 1>(std::forward<Ts>(ts)...);
    }

    template<typename... T>
    constexpr typename std::common_type<T...>::type sum(T&&... values) {
        typename std::common_type<T...>::type sum_{ };
        static_for<sizeof...(T)>([&](auto index) {
            sum_ += get<index>(values...);
        });
        return sum_;
    }
}; // end namespace inlay

// int main() {
//     constexpr auto sum1 = inlay::sum(1, 2.5, 4);
//     std::cout << sum1 << std::endl;
//
//     using namespace std::string_literals;
//     auto sum2 = inlay::sum("Hello "s, "world");
//     std::cout << sum2 << std::endl;
// }
