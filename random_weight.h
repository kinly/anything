#pragma once
#include <cassert>
#include <numeric>
#include <queue>
#include <random>

namespace inlay::random {

    static std::default_random_engine& engine() {
        static thread_local std::default_random_engine mt(static_cast<uint32_t>(std::time(nullptr)));
        return mt;
    }

    template <class _T1, class _T2, typename std::enable_if<!std::is_floating_point<_T1>::value && !std::is_floating_point<_T2>::value, bool >::type = true>
    auto range(_T1 min_value, _T2 max_value) -> decltype(min_value + max_value) {
        if (min_value == max_value)
            return min_value;

        using Result = decltype(min_value + max_value);

        if (min_value > max_value)
            return std::uniform_int_distribution<Result>(max_value, min_value)(engine());
        return std::uniform_int_distribution<Result>(min_value, max_value)(engine());
    }

    template <class _T1, class _T2, typename std::enable_if<std::is_floating_point<_T1>::value || std::is_floating_point<_T2>::value, bool >::type = true>
    auto rangef(_T1 min_value, _T2 max_value) -> decltype(min_value + max_value) {
        if (min_value == max_value)
            return min_value;

        using Result = decltype(min_value + max_value);

        if (min_value > max_value)
            return std::uniform_real_distribution<Result>(max_value, min_value)(engine());
        return std::uniform_real_distribution<Result>(min_value, max_value)(engine());
    }

    /* Walker's Alias Method 代码内部做了权重修正 */
    template <class _Ty>
    class weight_faster_alias {
    private:
        using value_type = _Ty;
        const std::vector<value_type> _values;
        std::vector<std::pair<double, size_t>> _alias;
        mutable std::uniform_real_distribution<double> _real_dis{ 0.0, 1.0 };
        mutable std::uniform_int_distribution<size_t>  _int_dis;

    public:
        template <typename Iterator_V, typename Iterator_P>
        weight_faster_alias(const Iterator_V vs, const Iterator_V ve, const Iterator_P ps, const Iterator_P pe)
            : _values(vs, ve)
            ,_int_dis(0, std::distance(ps, pe) - 1) {
            assert(std::distance(vs, ve) != 0 && std::distance(vs, ve) == std::distance(ps, pe));

            std::vector<double> reprods;
            const double sum_prod = std::accumulate(ps, pe, 0.0);
            if (std::fabs(1.0 - sum_prod) > std::numeric_limits<double>::epsilon()) {
                for (auto iter = ps; iter != pe; ++iter) {
                    reprods.push_back(*iter / sum_prod);
                }
            } else {
                reprods.insert(reprods.end(), ps, pe);
            }
            const double re_sum_prod = std::accumulate(reprods.begin(), reprods.end(), 0.0);
            assert(std::fabs(1.0 - re_sum_prod) < std::numeric_limits<double>::epsilon());
            _alias = generate_alias_table(reprods.begin(), reprods.end());
        }

        value_type operator()() const {
            const size_t idx = _int_dis(engine());
            if (_real_dis(engine()) >= _alias[idx].first
                && _alias[idx].second != std::numeric_limits<size_t>::max()) {
                return _values[_alias[idx].second];
            } else {
                return _values[idx];
            }
        }

    private:
        template <typename Iterator_RP>
        std::vector<std::pair<double, size_t>> generate_alias_table(const Iterator_RP ps, const Iterator_RP pe) {
            const size_t sz = std::distance(ps, pe);
            std::vector<std::pair<double, size_t>> alias(sz, { 0.0, std::numeric_limits<size_t>::max() });
            std::queue<size_t>  small, large;

            auto iter = ps;
            size_t i = 0;
            for (; iter != pe; ++iter, ++i) {
                alias[i].first = sz * (*iter);
                if (alias[i].first < 1.0) {
                    small.push(i);
                } else {
                    large.push(i);
                }
            }

            while (!(small.empty()) and !(large.empty())) {
                auto s = small.front(), l = large.front();
                small.pop(), large.pop();
                alias[s].second = l;
                alias[l].first -= (1.0 - alias[s].first);

                if (alias[l].first < 1.0) {
                    small.push(l);
                } else {
                    large.push(l);
                }
            }

            return alias;
        }
    };

    /* Expansion 适合总和较小的整数权重，空间换时间，这个最快 */
    template<class _Ty, class _Prods = uint32_t>
    class weight_faster_expansion {
    private:
        using value_type = _Ty;
        using prod_type = _Prods;
        std::vector<value_type> _values;
    public:
        template <typename Iterator_V, typename Iterator_P>
        weight_faster_expansion(const Iterator_V vs, const Iterator_V ve, const Iterator_P ps, const Iterator_P pe) {
            assert(std::distance(vs, ve) != 0 && std::distance(vs, ve) == std::distance(ps, pe));
            const prod_type sum_prod = std::accumulate(ps, pe, 0);
            auto iter_v = vs;
            auto iter_p = ps;
            for (; iter_v != ve && iter_p != pe; ++iter_v, ++iter_p) {
                _values.insert(_values.end(), *iter_p, *iter_v);
            }
        }

        value_type operator()() const {
            const size_t idx = range(0, _values.size() - 1);
            return _values[idx];
        }
    };

    /* binary 一般性权重随机 */
    template<class _Ty, class _Prods = uint32_t>
    class weight_faster_binary {
    private:
        using value_type = _Ty;
        using prod_type = _Prods;
        std::vector<value_type> _values;
        std::vector<prod_type> _prods;
    public:
        template <typename Iterator_V, typename Iterator_P>
        weight_faster_binary(const Iterator_V vs, const Iterator_V ve, const Iterator_P ps, const Iterator_P pe) {
            assert(std::distance(vs, ve) != 0 && std::distance(vs, ve) == std::distance(ps, pe));

            _values.insert(_values.end(), vs, ve);

            prod_type last_sum = 0;
            for (auto iter = ps; iter != pe; ++iter) {
                last_sum += *iter;
                _prods.push_back(last_sum);
            }
        }

        value_type operator()() const {
            const auto rand = range(1, _prods.back());
            auto idx = std::lower_bound(_prods.begin(), _prods.end(), rand) - _prods.begin();
            return _values[idx];
        }
    };

}; // end namespace inlay::random

/*
 *
template<class _Period = std::micro>
class timer_cost {
    std::chrono::steady_clock::time_point _last_time_point;
public:
    timer_cost()
        : _last_time_point(std::chrono::steady_clock::now()) {
    }

    void print(const std::string& msg) {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double, _Period> diff{ now - _last_time_point };
        std::cout << msg << " " << diff.count() << std::endl;
        _last_time_point = std::chrono::steady_clock::now();
    }
};
 *    {
        std::vector<int> v{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::vector<uint32_t> p{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::vector<double> dp{ 1 / 55.0, 2 / 55.0, 3 / 55.0, 4 / 55.0, 5 / 55.0, 6 / 55.0, 7 / 55.0, 8 / 55.0, 9 / 55.0 };
        inlay::random::weight_faster_alias<int> wfa(v.begin(), v.end(), dp.begin(), dp.end());
        inlay::random::weight_faster_alias<int> wfa_2(v.begin(), v.end(), p.begin(), p.end());
        inlay::random::weight_faster_expansion<int> wfs(v.begin(), v.end(), p.begin(), p.end());
        inlay::random::weight_faster_binary<int> wfb(v.begin(), v.end(), p.begin(), p.end());

        std::map<int, int> wfa_statistics;
        std::map<int, int> wfa_2_statistics;
        std::map<int, int> wfs_statistics;
        std::map<int, int> wfb_statistics;

        timer_cost tc;

        for (int i = 0; i < 40000; ++i) {
            auto wfa_index = wfa();
            wfa_statistics[wfa_index] += 1;
        }
        tc.print("wfa:");

        for (int i = 0; i < 40000; ++i) {
            auto wfa_2_index = wfa_2();
            wfa_2_statistics[wfa_2_index] += 1;
        }
        tc.print("wfa_2:");

        for (int i = 0; i < 40000; ++i) {
            auto wfs_index = wfs();
            wfs_statistics[wfs_index] += 1;
        }
        tc.print("wfs:");

        for (int i = 0; i < 40000; ++i) {
            auto wfb_index = wfb();
            wfb_statistics[wfb_index] += 1;
        }
        tc.print("wfb:");

        // break-point
        int i = 0;
        i += 1;
    }
 *
 */
