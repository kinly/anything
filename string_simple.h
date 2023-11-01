#pragma once
#include <algorithm>
#include <istream>
#include <sstream>
#include <string>
#include <vector>

namespace base {

    template<class ret_tt, std::enable_if_t<std::is_integral_v<ret_tt>&& std::is_unsigned_v<ret_tt> == false, bool> = true>
    ret_tt str2numeric(const std::string_view sv, char** nxt = nullptr, int radix = 10) {
        return static_cast<ret_tt>(strtoll(sv.data(), nxt, radix));
    }

    template<class ret_tt, std::enable_if_t<std::is_integral_v<ret_tt>&& std::is_unsigned_v<ret_tt> != false, bool> = true>
    ret_tt str2numeric(const std::string_view sv, char** nxt = nullptr, int radix = 10) {
        return static_cast<ret_tt>(strtoull(sv.data(), nxt, radix));
    }

    template<class ret_tt, std::enable_if_t<std::is_floating_point_v<ret_tt>, bool> = true>
    ret_tt str2numeric(const std::string_view sv, char** nxt = nullptr, int radix = 10) {
        return static_cast<ret_tt>(strtod(sv.data(), nxt));
    }

    inline std::string_view ltrim(std::string_view sv) {
        std::string::size_type index = 0;
        for (auto iter = sv.cbegin(); iter != sv.cend(); ++iter) {
            if (std::isspace(*iter))
                ++index;
            else
                break;
        }
        sv.remove_prefix(index);
        return sv;
    }

    inline std::string_view rtrim(std::string_view sv) {
        std::string::size_type index = 0;
        for (auto iter = sv.crbegin(); iter != sv.crend(); ++iter) {
            if (std::isspace(*iter))
                ++index;
            else
                break;
        }
        sv.remove_suffix(index);
        return sv;
    }

    inline std::string_view trim(std::string_view sv) {
        return rtrim(ltrim(sv));
    }

    template<template<class, class> class container_tt = std::vector, template<class> class alloc_tt = std::allocator>
    container_tt<std::string, alloc_tt<std::string>> split(std::string_view sv, char delim) {
        std::stringstream ss(sv.data());
        std::string item;
        container_tt<std::string, alloc_tt<std::string>> elems;
        while (std::getline(ss, item, delim)) {
            elems.emplace_back(std::move(item));
        }
        return elems;
    }

    template<template<class, class> class container_tt = std::vector, template<class> class alloc_tt = std::allocator>
    container_tt<std::string, alloc_tt<std::string>> split(std::string_view sv, std::string_view delim) {
        std::size_t start_pos = 0;
        std::size_t end_pos;
        std::size_t delim_len = delim.size();
        container_tt<std::string, alloc_tt<std::string>> elems;
        while ((end_pos = sv.find(delim, start_pos)) != std::string::npos) {
            std::string_view item = sv.substr(start_pos, end_pos - start_pos);
            start_pos = end_pos + delim_len;
            elems.emplace_back(item);
        }
        if (sv.size() > start_pos)
            elems.emplace_back(sv.substr(start_pos));
        return elems;
    }

    inline std::string replace_all(std::string str, const std::string& search, const std::string& replace) {
        std::string::size_type pos = 0;
        while ((pos = str.find(search, pos)) != std::string::npos) {
            str.replace(pos, search.length(), replace);
            pos += replace.length();
        }
        return str;
    }

    inline std::string replace_all_ic(std::string str, const std::string& search, const std::string& replace) {
        std::string lower_str = str;
        std::string lower_search = search;
        std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
        std::transform(lower_search.begin(), lower_search.end(), lower_search.begin(), ::tolower);

        std::string::size_type pos = 0;
        while ((pos = lower_str.find(lower_search)) != std::string::npos) {
            str.replace(pos, lower_search.length(), replace);
            lower_str.replace(pos, lower_search.length(), lower_search);
            pos += replace.length();
        }
        return str;
    }

    // compare const, safe signature compare function (see: Timing Attack)
    // size: std::min(a.length, b.length)
    inline bool compare_const(std::string_view lsv, std::string_view rsv) {
        if (lsv.empty() || rsv.empty()) return false;

        unsigned char result = 0;
        const auto size = std::max(lsv.size(), rsv.size());
        for (std::size_t i = 0; i < size; i++) {
            result |= lsv[i] ^ rsv[i];
        }
        return result == 0; /* returns 0 if equal, nonzero otherwise */
    }

    // via: https://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf/6089413#6089413
    inline std::istream& rewrite_getline(std::istream& is, std::string& to) {
        to.clear();

        std::istream::sentry ise(is, true);
        std::streambuf* sb = is.rdbuf();

        for (;;) {
            switch (auto cc = sb->sbumpc()) {
            case '\n':
                return is;
            case '\r':
                if (sb->sgetc() == '\n')
                    sb->sbumpc();
                return is;
            case std::streambuf::traits_type::eof():
                // Also handle the case when the last line has no line ending
                if (to.empty())
                    is.setstate(std::ios::eofbit);
                return is;
            default:
                to += static_cast<char>(cc);
            }
        }
    }
}; // end namespace base
