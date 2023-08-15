#pragma once
#include <string>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) // windows platform
#include <windows.h>
#else // other platforms
#include <cstdlib>
#include <locale.h>
#endif // defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

namespace encoding {

    static inline uint64_t mbs2wcs(const char* from, wchar_t* to, uint64_t max_target_len, uint32_t code_page /* = 65001 */) {
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
        int32_t ret = MultiByteToWideChar(code_page, 0, from, -1, to, static_cast<int32_t>(max_target_len));
        if (ret <= 0)
        {
            return -1;
        }
        return ret - 1;
#else // other platforms
        std::lock_guard<std::mutex> lock_guard(m_lock);
        setlocale(LC_CTYPE, "");
        const char** p_from = &from;
        mbstate_t st = mbstate_t();
        uint64_t ret = (uint64_t)mbsrtowcs(to, p_from, max_target_len, &st);

        //uint64_t ret = (uint64_t)mbsrtowcs(to, from, max_target_len);
        setlocale(LC_CTYPE, "C");
        return ret;
#endif
    }

    static inline uint64_t wcs2mbs(const wchar_t* from, char* to, uint64_t max_target_len, uint32_t code_page /* = 65001 */) {
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
        int32_t ret = WideCharToMultiByte(code_page, 0, from, -1, to, static_cast<int32_t>(max_target_len), nullptr, nullptr);
        if (ret <= 0)
        {
            return -1;
        }
        return ret - 1;
#else // other platforms
        std::lock_guard<std::mutex> lock_guard(m_lock);
        setlocale(LC_CTYPE, "");
        const wchar_t** p_from = &from;
        mbstate_t st = mbstate_t();
        uint64_t ret = (uint64_t)wcsrtombs(to, p_from, max_target_len, &st);

        //uint64_t ret = (uint64_t)wcstombs(to, from, max_target_len);
        setlocale(LC_CTYPE, "C");
        return ret;
#endif
    }

    static inline std::wstring __to_wstring(const std::string& src, uint32_t code_page) {
        static std::wstring empty_result = L"";

        if (src.length() == 0) {
            return empty_result;
        }

        const uint64_t target_size_in_characters = mbs2wcs(src.data(), nullptr, 0, code_page);
        if (target_size_in_characters == static_cast<uint64_t>(-1)) {
            return empty_result;
        }

        wchar_t* tmp = new(std::nothrow) wchar_t[(size_t)target_size_in_characters + 1]();
        if (!tmp) {
            return empty_result;
        }

        mbs2wcs(src.data(), tmp, target_size_in_characters + 1, code_page);
        tmp[(size_t)target_size_in_characters] = L'\0';

        std::wstring ret_str(tmp);

        delete[] tmp;
        tmp = nullptr;

        return ret_str;
    }

    static inline std::string __to_string(const std::wstring& src, uint32_t code_page) {
        static std::string empty_result = "";

        if (src.length() == 0) {
            return empty_result;
        }

        const uint64_t target_size_in_characters = wcs2mbs(src.data(), nullptr, 0, code_page);
        if (target_size_in_characters == static_cast<uint64_t>(-1)) {
            return empty_result;
        }

        char* tmp = new(std::nothrow) char[(size_t)target_size_in_characters + 1]();
        if (!tmp) {
            return empty_result;
        }

        wcs2mbs(src.data(), tmp, target_size_in_characters + 1, code_page);
        tmp[(size_t)target_size_in_characters] = L'\0';

        std::string ret_str(tmp);

        delete[] tmp;
        tmp = nullptr;

        return ret_str;
    }

    // code_page: 65001(CP_UTF8), 0(CP_ACP)

    static std::string gbk2utf8(const std::string& src) {
        return __to_string(__to_wstring(src, 0), 65001);
    }

    static std::string utf82gbk(const std::string& src) {
        return __to_string(__to_wstring(src, 65001), 0);
    }

}; // end namespace encoding
