#pragma once
#include <iterator>
#include <map>
#include <memory>
#include <unordered_map>

namespace sort {

    template <typename T, typename = std::void_t<>>
    struct is_std_hash_able : std::false_type {};

    template <typename T>
    struct is_std_hash_able<T, std::void_t<decltype(std::declval<std::hash<T>>()(std::declval<T>()))>> : std::true_type {};

    template <typename T>
    constexpr bool is_std_hash_able_v = is_std_hash_able<T>::value;

    template<
        class _Key, class _Value_key, class _Value_data,
        std::enable_if_t<is_std_hash_able_v<_Value_key>, bool> = true
    >
    class sort final {
        using score_type = _Key;
        using element_key = _Value_key;
        using element_value = _Value_data;

        using sorted_map = std::map<score_type, std::unordered_map<element_key, element_value>>;
        using elements_map = std::unordered_map<element_key, typename sorted_map::iterator>;

    private:
        sorted_map _sorted;
        elements_map _elements;

    public:
        sort() = default;
        ~sort() = default;

        void put(const element_key& ele_key, const element_value& ele, const score_type& score) {
            rem(ele_key);

            auto iter = _sorted.insert({ score, {} });
            iter.first->second[ele_key] = ele;
            _elements[ele_key] = iter.first;
        }

        void rem(const element_key& ele_key) {
            const auto iter = _elements.find(ele_key);
            if (iter == _elements.end()) {
                return;
            }
            // sorted_map's iterator->second.earse...
            iter->second->second.erase(ele_key);
            if (iter->second->second.empty()) {
                _sorted.erase(iter->second);
            }
            _elements.erase(ele_key);
        }

        int rank(const element_key& ele_key) {
            static constexpr int error_result = -1;
            const auto iter = _elements.find(ele_key);
            if (iter == _elements.end()) {
                return error_result;
            }
            int result = 0;
            for (auto iter_sort = _sorted.begin(); iter_sort != iter->second; ++iter_sort) {
                result += iter_sort->second.size();
            }
            return result + 1;
        }

        int revrank(const element_key& ele_key) {
            static constexpr int error_result = -1;
            const auto iter = _elements.find(ele_key);
            if (iter == _elements.end()) {
                return error_result;
            }
            int result = 0;
            for (auto iter_sort = iter->second; iter_sort != _sorted.end(); ++iter_sort) {
                result += iter_sort->second.size();
            }
            return result;
        }

        std::vector<element_value> range(int start, int stop) {
            auto iter_s = _sorted.end();
            auto iter_e = _sorted.end();
            if (start >= 0) {
                iter_s = std::next(_sorted.begin(), start);
            } else {
                iter_s = std::next(_sorted.end(), start + 1);
            }
            if (stop >= 0) {
                iter_e = std::next(_sorted.begin(), stop);
            } else {
                iter_e = std::next(_sorted.end(), stop + 1);
            }

            std::vector<element_value> result;
            for (auto iter = iter_s; iter != _sorted.end(); ++iter) {
                std::transform(
                    iter->second.begin(),
                    iter->second.end(),
                    std::back_inserter(result),
                    [](auto& kv) { return kv.second; }
                );
                if (iter == iter_e) break;
            }
            return result;
        }

        std::vector<element_value> revrange(int start, int stop) {
            auto iter_s = _sorted.rbegin();
            auto iter_e = _sorted.rbegin();
            if (start >= 0) {
                iter_s = std::next(_sorted.rbegin(), start);
            } else {
                iter_s = std::next(_sorted.rend(), start + 1);
            }
            if (stop >= 0) {
                iter_e = std::next(_sorted.rbegin(), stop);
            } else {
                iter_e = std::next(_sorted.rend(), stop + 1);
            }

            std::vector<element_value> result;
            for (auto iter = iter_s; iter != _sorted.rend(); ++iter) {
                std::transform(
                    iter->second.begin(),
                    iter->second.end(),
                    std::back_inserter(result),
                    [](auto& kv) { return kv.second; }
                );
                if (iter == iter_e) break;
            }
            return result;
        }
    };
}; // end namespace sort


/*struct sort_key {
    uint64_t score;
    uint64_t timestamp;

    bool operator < (const sort_key& tar) const {
        return score < tar.score
            || (score == tar.score && timestamp < tar.timestamp);
    }
};

struct sort_value {
    uint64_t uuid;
    std::string name;
};


sort::sort<sort_key, uint64_t, sort_value> _sort_1;

_sort_1.put(1, { 1, "_1" }, { 1, 0 });
_sort_1.put(2, { 2, "_2" }, { 2, 0 });
_sort_1.put(3, { 3, "_3" }, { 3, 0 });
_sort_1.put(4, { 4, "_4" }, { 4, 0 });
_sort_1.put(5, { 5, "_5" }, { 5, 0 });
_sort_1.put(6, { 6, "_6" }, { 6, 0 });
_sort_1.put(7, { 7, "_7" }, { 7, 0 });
_sort_1.put(8, { 8, "_8" }, { 8, 0 });
_sort_1.put(9, { 9, "_9" }, { 9, 0 });

std::cout << "rank 1: " << _sort_1.rank(1) << std::endl;
std::cout << "rank 2: " << _sort_1.rank(2) << std::endl;
std::cout << "rank 3: " << _sort_1.rank(3) << std::endl;
std::cout << "rank 4: " << _sort_1.rank(4) << std::endl;
std::cout << "rank 5: " << _sort_1.rank(5) << std::endl;
std::cout << "rank 6: " << _sort_1.rank(6) << std::endl;
std::cout << "rank 7: " << _sort_1.rank(7) << std::endl;
std::cout << "rank 8: " << _sort_1.rank(8) << std::endl;
std::cout << "rank 9: " << _sort_1.rank(9) << std::endl;
std::cout << "revrank 1: " << _sort_1.revrank(1) << std::endl;
std::cout << "revrank 2: " << _sort_1.revrank(2) << std::endl;
std::cout << "revrank 3: " << _sort_1.revrank(3) << std::endl;
std::cout << "revrank 4: " << _sort_1.revrank(4) << std::endl;
std::cout << "revrank 5: " << _sort_1.revrank(5) << std::endl;
std::cout << "revrank 6: " << _sort_1.revrank(6) << std::endl;
std::cout << "revrank 7: " << _sort_1.revrank(7) << std::endl;
std::cout << "revrank 8: " << _sort_1.revrank(8) << std::endl;
std::cout << "revrank 9: " << _sort_1.revrank(9) << std::endl;

auto range_res = _sort_1.range(0, -1);
auto revrange_res = _sort_1.revrange(0, -1);

auto range_res2 = _sort_1.range(0, 5);
auto revrange_res2 = _sort_1.revrange(0, 5);

// break-point
int i = 0;
i += 1;
*/
