#pragma once
#include <array>
#include <string>
#include <chrono>
#include <iomanip>

namespace easy { namespace datetime {

    using std::chrono::microseconds;
    using std::chrono::milliseconds;
    using std::chrono::seconds;
    using std::chrono::minutes;
    using std::chrono::hours;

    using days = std::chrono::duration<long long, std::ratio_multiply<std::ratio<24>, hours::period>>;

    using clock = std::chrono::system_clock;
    using default_duration = std::chrono::milliseconds;

    using timestamp = std::time_t;

    /// @brief current(now) time_point
    /// @tparam _Duration offset duration type
    /// @param offset duration value
    /// @return time_point (system_clock)
    template<typename _Duration = default_duration>
    static constexpr inline clock::time_point current_time_point(long long offset = 0) noexcept {
        return clock::now() + _Duration(offset);
    }

    /// @brief current(now) time_point
    /// @tparam _Rep an arithmetic type representing the number of ticks
    /// @tparam _Period a std::ratio representing the tick period (i.e. the number of second's fractions per tick)
    /// @param offset offset value
    /// @return time_point (system_clock)
    template<typename _Rep = long long, typename _Period = std::milli>
    static constexpr inline clock::time_point current_time_point(std::chrono::duration<_Rep, _Period> offset) noexcept {
        return current_time_point(std::chrono::duration<_Rep, _Period>(offset.count()));
    }

    /// @brief current(now) timestamp
    /// @tparam _Duration duration type
    /// @return timestamp of duration's ratio
    template<typename _Duration = default_duration>
    static constexpr inline timestamp current_timestamp() noexcept {
        return std::chrono::duration_cast<_Duration>(current_time_point<_Duration>().time_since_epoch()).count();
    }

    /// @brief relative(now) timestamp
    /// @tparam _Duration duration type
    /// @tparam _Rep an arithmetic type representing the number of ticks
    /// @tparam _Period a std::ratio representing the tick period (i.e. the number of second's fractions per tick)
    /// @param offset offset value
    /// @return timestamp of duration's ratio
    template<typename _Duration = default_duration, typename _Rep = long long, typename _Period = std::milli>
    static constexpr inline timestamp relative_timestamp(std::chrono::duration<_Rep, _Period> offset) noexcept {
        return std::chrono::duration_cast<_Duration>(
            current_time_point(offset).time_since_epoch()
            ).count();
    }

    /// @brief current(now) timestamp of day start 
    /// @tparam _Duration duration type
    /// @return timestamp of duration's ratio
    template<typename _Duration = default_duration>
    static constexpr inline timestamp current_days_timestamp() noexcept {
        return std::chrono::duration_cast<_Duration>(
            std::chrono::duration_cast<days>(current_time_point<_Duration>().time_since_epoch())
            ).count();
    }

    /// @brief relative(now) timestamp of day start 
    /// @tparam _Duration duration type
    /// @tparam _Rep an arithmetic type representing the number of ticks
    /// @tparam _Period a std::ratio representing the tick period (i.e. the number of second's fractions per tick)
    /// @param offset offset value
    /// @return timestamp of duration's ratio
    template<typename _Duration = default_duration, typename _Rep = long long, typename _Period = std::milli>
    static constexpr inline timestamp relative_days_timestamp(std::chrono::duration<_Rep, _Period> offset) noexcept {
        return std::chrono::duration_cast<_Duration>(
            std::chrono::duration_cast<days>(current_time_point(offset).time_since_epoch())
            ).count();
    }

    /// @brief current(now) timestamp of hour start
    /// @tparam _Duration duration type
    /// @return timestamp of duration's ratio
    template<typename _Duration = default_duration>
    static constexpr inline timestamp current_hours_timestamp() noexcept {
        return std::chrono::duration_cast<_Duration>(
            std::chrono::duration_cast<hours>(current_time_point<_Duration>().time_since_epoch())
            ).count();
    }

    /// @brief relative(now) timestamp of hour start 
    /// @tparam _Duration duration type
    /// @tparam _Rep an arithmetic type representing the number of ticks
    /// @tparam _Period a std::ratio representing the tick period (i.e. the number of second's fractions per tick)
    /// @param offset offset value
    /// @return timestamp of duration's ratio
    template<typename _Duration = default_duration, typename _Rep = long long, typename _Period = std::milli>
    static constexpr inline timestamp relative_hours_timestamp(std::chrono::duration<_Rep, _Period> offset) noexcept {
        return std::chrono::duration_cast<_Duration>(
            std::chrono::duration_cast<hours>(current_time_point(offset).time_since_epoch())
            ).count();
    }

    /// @brief current(now) timestamp of minutes start
    /// @tparam _Duration duration type
    /// @return timestamp of duration's ratio
    template<typename _Duration = default_duration>
    static constexpr inline timestamp current_minutes_timestamp() noexcept {
        return std::chrono::duration_cast<_Duration>(
            std::chrono::duration_cast<minutes>(current_time_point<_Duration>().time_since_epoch())
            ).count();
    }

    /// @brief relative(now) timestamp of minutes start 
    /// @tparam _Duration duration type
    /// @tparam _Rep an arithmetic type representing the number of ticks
    /// @tparam _Period a std::ratio representing the tick period (i.e. the number of second's fractions per tick)
    /// @param offset offset value
    /// @return timestamp of duration's ratio
    template<typename _Duration = default_duration, typename _Rep = long long, typename _Period = std::milli>
    static constexpr inline timestamp relative_minutes_timestamp(std::chrono::duration<_Rep, _Period> offset) noexcept {
        return std::chrono::duration_cast<_Duration>(
            std::chrono::duration_cast<minutes>(current_time_point(offset).time_since_epoch())
            ).count();
    }

    /// @brief timestamp to time_point(system clock)
    /// @tparam _Duration duration type
    /// @param ts 
    /// @return
    template<typename _Duration = default_duration>
    static constexpr clock::time_point from_timestamp(timestamp ts) {
        return clock::time_point(_Duration(ts));
    }


    // ... low performance ...
    // static constexpr char default_format[] = "%Y-%m-%d %H:%M:%S";
    // static constexpr char default_format_ms[] = "%Y-%m-%d %H:%M:%S.%X";
    // static inline std::string local_time_string(clock::time_point ctp, bool format_ms) {
    //
    //     const std::time_t time_t_ = std::chrono::system_clock::to_time_t(ctp);
    //
    //     const std::tm* tm = std::localtime(&time_t_);
    //     if (tm == nullptr) return "";
    //
    //     std::ostringstream ss;
    //     ss << std::put_time(tm, default_format);
    //     if (format_ms) {
    //         static constexpr long long ms_ratio = 1000;
    //         const long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(ctp.time_since_epoch()).count() % ms_ratio;
    //         ss << std::setw(3) << std::setfill('0') << ms;
    //     }
    //     return ss.str();
    // }

    /// @brief convert to localtime string_view
    /// @param ctp source 
    /// @param format_ms 
    /// @return 
    static inline std::string_view localtime_string_v(clock::time_point ctp, bool format_ms = true) {

        static thread_local bool been_set = false;
        static thread_local std::chrono::seconds last_secs{ };

        static constexpr std::size_t result_size = 32;
        static constexpr std::size_t secs_result_size = 19;         // length after format seconds
        static constexpr std::size_t msecs_result_size = 19 + 4;    // length after format seconds + msecs

        static constexpr long long ms_ratio = 1000;

        // %4d-%02d-%02d %02d:%02d:%02d.%03d
        static thread_local char last_secs_result[result_size]{ };

        std::chrono::system_clock::duration ctp_dur = ctp.time_since_epoch();
        std::chrono::seconds cpt_secs = std::chrono::duration_cast<std::chrono::seconds>(ctp_dur);

        if (been_set && last_secs == cpt_secs) {
            std::string_view result(last_secs_result, result_size);
            if (format_ms) {
                const long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(ctp_dur).count() % ms_ratio;
                [[maybe_unused]] auto snf_res_ms = snprintf(last_secs_result + secs_result_size, result_size - secs_result_size - 1, ".%03lld", ms);
                result.remove_suffix(result_size - msecs_result_size);
            } else {
                result.remove_suffix(result_size - secs_result_size);
            }
            return result;
        }

        last_secs = cpt_secs;
        been_set = true;

        static thread_local std::tm cpt_tm = std::tm();
        auto cpt_secs_count = last_secs.count();

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) // windows platform
        [[maybe_unused]] auto lt_res = localtime_s(&cpt_tm, &cpt_secs_count);
#else // other platforms
        [[maybe_unused]] auto lt_res = localtime_r(&cpt_secs_count, &cpt_tm);
#endif

        [[maybe_unused]] auto snf_res = snprintf(last_secs_result, result_size - 1, "%4d-%02d-%02d %02d:%02d:%02d",
            cpt_tm.tm_year + 1900,
            cpt_tm.tm_mon + 1,
            cpt_tm.tm_mday,
            cpt_tm.tm_hour,
            cpt_tm.tm_min,
            cpt_tm.tm_sec);

        std::string_view result(last_secs_result, result_size);
        if (format_ms) {
            const long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(ctp_dur).count() % ms_ratio;
            [[maybe_unused]] auto snf_res_ms = snprintf(last_secs_result + secs_result_size, result_size - secs_result_size - 1, ".%03lld", ms);
            result.remove_suffix(result_size - msecs_result_size);
        } else {
            result.remove_suffix(result_size - secs_result_size);
        }
        return result;
    }

    /// @brief convert to localtime string
    /// @param ctp source 
    /// @param format_ms 
    /// @return 
    static inline std::string localtime_string(clock::time_point ctp, bool format_ms = true) {
        const auto sv = localtime_string_v(ctp, format_ms);
        return std::string{ sv.data(), sv.size() };
    }

    /// @brief convert to utc string_view
    /// @param ctp source 
    /// @param format_ms 
    /// @return 
    static inline std::string_view utc_string_v(clock::time_point ctp, bool format_ms = true) {

        static thread_local bool been_set = false;
        static thread_local std::chrono::seconds last_secs{ };

        static constexpr std::size_t result_size = 32;
        static constexpr std::size_t secs_result_size = 19;         // length after format seconds
        static constexpr std::size_t msecs_result_size = 19 + 4;    // length after format seconds + msecs

        static constexpr long long ms_ratio = 1000;

        // %4d-%02d-%02d %02d:%02d:%02d.%03d
        static thread_local char last_secs_result[result_size]{ };

        std::chrono::system_clock::duration ctp_dur = ctp.time_since_epoch();
        std::chrono::seconds cpt_secs = std::chrono::duration_cast<std::chrono::seconds>(ctp_dur);

        if (been_set && last_secs == cpt_secs) {
            std::string_view result(last_secs_result, result_size);
            if (format_ms) {
                const long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(ctp_dur).count() % ms_ratio;
                [[maybe_unused]] auto snf_res_ms = snprintf(last_secs_result + secs_result_size, result_size - secs_result_size - 1, ".%03lld", ms);
                result.remove_suffix(result_size - msecs_result_size);
            } else {
                result.remove_suffix(result_size - secs_result_size);
            }
            return result;
        }

        last_secs = cpt_secs;
        been_set = true;

        static thread_local std::tm cpt_tm = std::tm();
        auto cpt_secs_count = last_secs.count();

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) // windows platform
        [[maybe_unused]] auto lt_res = gmtime_s(&cpt_tm, &cpt_secs_count);
#else // other platforms
        [[maybe_unused]] auto lt_res = gmtime_r(&cpt_secs_count, &cpt_tm);
#endif

        [[maybe_unused]] auto snf_res = snprintf(last_secs_result, result_size - 1, "%4d-%02d-%02d %02d:%02d:%02d",
            cpt_tm.tm_year + 1900,
            cpt_tm.tm_mon + 1,
            cpt_tm.tm_mday,
            cpt_tm.tm_hour,
            cpt_tm.tm_min,
            cpt_tm.tm_sec);

        std::string_view result(last_secs_result, result_size);
        if (format_ms) {
            const long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(ctp_dur).count() % ms_ratio;
            [[maybe_unused]] auto snf_res_ms = snprintf(last_secs_result + secs_result_size, result_size - secs_result_size - 1, ".%03lld", ms);
            result.remove_suffix(result_size - msecs_result_size);
        } else {
            result.remove_suffix(result_size - secs_result_size);
        }
        return result;
    }

    /// @brief convert to utc string
    /// @param ctp source 
    /// @param format_ms 
    /// @return 
    static inline std::string utc_string(clock::time_point ctp, bool format_ms = true) {
        const auto sv = utc_string_v(ctp, format_ms);
        return std::string{ sv.data(), sv.size() };
    }

    /// @brief complex exchange to time_point
    /// @tparam _Future_Point 
    /// @param source 
    /// @return
    /// 
    /// 2021-03-03 11:22:33.12 -> millisecs = 12 time_point
    /// 2021-03-03 11:22:33    -> millisecs = 0  time_point
    /// 2021-03-03 11:22       -> seconds = 0    time_point
    /// 2021-03-03 11          -> minutes = 0    time_point
    /// 2021-03-03             -> hours = 0      time_point
    /// (next day if _Future_Point == true && result time_point < current_time_point)
    ///  ||
    ///  ……
    /// 01:02:03.456 -> current date & 01:02:03.456 time_point
    /// 01:02:03     -> current date & 01:02:03     time_point
    template<bool _Future_Point = false>
    static inline std::pair<clock::time_point, bool> complex_time_point(const char* source) {

        std::string_view sv(source);

        bool days_next = _Future_Point;

        std::chrono::milliseconds convert_msecs = {};
        int convert_year = 0;
        int convert_month = 0;
        int convert_day = 0;

        clock::time_point current_ctp = current_time_point();

        do {
            if (sv.empty()) {
                break;
            }

            static constexpr char ms_split_char = '.';
            static constexpr char time_split_char = ':';
            static constexpr char empty_split_char = ' ';
            static constexpr char date_split_char = '-';

            static constexpr std::size_t max_array_size = 8;

            std::array<char, max_array_size> last_arr{ };
            std::size_t last_size = 0;

            auto lasts_reset = [&last_arr, &last_size]() {
                last_arr.fill(0);
                last_size = 0;
            };

            lasts_reset();

            static thread_local std::vector<int> result_ms{ };
            static thread_local std::vector<int> result_time{ };
            static thread_local std::vector<int> result_date{ };
            result_ms.reserve(3); result_ms.clear();
            result_time.reserve(3); result_time.clear();
            result_date.reserve(3); result_date.clear();

            auto* result_end = &result_ms;

            static constexpr std::size_t ok_date_size = 3;

            auto iter = sv.begin();
            while (iter != sv.end()) {

                if (last_size == max_array_size)
                    return { clock::time_point{ }, false };

                switch (*iter) {
                case ms_split_char: {
                    result_end = &result_ms;

                    result_time.emplace_back(strtol(last_arr.data(), nullptr, 10));
                    lasts_reset();
                }
                break;
                case empty_split_char: {
                    result_end = &result_time;

                    result_date.emplace_back(strtol(last_arr.data(), nullptr, 10));
                    lasts_reset();
                }
                break;
                case time_split_char: {
                    result_end = &result_time;

                    result_time.emplace_back(strtol(last_arr.data(), nullptr, 10));
                    lasts_reset();
                }
                break;
                case date_split_char: {
                    result_end = &result_date;

                    result_date.emplace_back(strtol(last_arr.data(), nullptr, 10));
                    lasts_reset();
                }
                break;
                default:
                    last_arr[last_size++] = *iter;
                    break;
                }

                if (++iter == sv.end()) {
                    result_end->emplace_back(strtol(last_arr.data(), nullptr, 10));
                }
            }

            static constexpr std::array<long long, 3> time_to_msecs{ 3600 * 1000ll, 60 * 1000ll, 1000ll};

            if (!result_ms.empty()) {
                convert_msecs += std::chrono::milliseconds(result_ms.at(0));
            }

            auto time_iter = result_time.begin();
            for (const auto& one : time_to_msecs) {
                if (time_iter == result_time.end()) 
                    break;

                convert_msecs += std::chrono::milliseconds((*time_iter) * one);
                ++time_iter;
            }


            if (result_date.size() == ok_date_size) {
                convert_year = result_date.at(0);
                convert_month = result_date.at(1);
                convert_day = result_date.at(2);

                days_next &= false;
            } else {
                std::tm cpt_tm = std::tm();
                auto cpt_secs_count = std::chrono::duration_cast<std::chrono::seconds>(current_ctp.time_since_epoch()).count();

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) // windows platform
                [[maybe_unused]] auto lt_res = localtime_s(&cpt_tm, &cpt_secs_count);
#else // other platforms
                [[maybe_unused]] auto lt_res = localtime_r(&cpt_secs_count, &cpt_tm);
#endif

                convert_day = cpt_tm.tm_mday;
                convert_month = cpt_tm.tm_mon + 1;
                convert_year = cpt_tm.tm_year + 1900;

                days_next &= true;
            }
        } while (false);

        std::tm convert_tm = std::tm();
        convert_tm.tm_year = convert_year - 1900;
        convert_tm.tm_mon = convert_month - 1;
        convert_tm.tm_mday = convert_day;

        clock::time_point ctp = clock::from_time_t(std::mktime(&convert_tm));

        if (days_next && ctp + convert_msecs < current_ctp) {
            return { ctp + convert_msecs + days(1), true };
        }
        return { ctp + convert_msecs, true };
    }

}}; // end namespace easy::datetime



/*
 *
std::cout << std::chrono::hours::period::num << std::endl;
std::cout << easy::datetime::days::period::num << std::endl;
std::cout << easy::datetime::current_days_timestamp() << std::endl;
{
    static thread_local char xx[32]{ "2021-03-03 11:22:33\0" };
    std::string xx_str = "2021-03-03 11:22:33";
    std::string_view format = "%Y-%m-%d %H:%M:%S";

    timer_cost cost;
    for (int i = 0; i < 10000; ++i) {
        get_time(xx_str, format);
    }
    cost.print("sys");

    for (int i = 0; i < 10000; ++i) {
        easy::datetime::complex_time_point(xx);
    }
    cost.print("easy");
}
{
    static thread_local char xx[32]{ "01:02:03.456\0" };

    auto ctp = easy::datetime::complex_time_point<true>(xx);
    std::cout << easy::datetime::localtime_string(ctp.first) << std::endl;
}
{
    static thread_local char xx[32]{ "5:0:0\0" };

    auto ctp = easy::datetime::complex_time_point<true>(xx);
    std::cout << easy::datetime::localtime_string(ctp.first) << std::endl;
}
{
    static thread_local char xx[32]{ "2021-01-2 11\0" };

    auto ctp = easy::datetime::complex_time_point<true>(xx);
    std::cout << easy::datetime::localtime_string(ctp.first) << std::endl;
}

std::cout << easy::datetime::localtime_string(easy::datetime::current_time_point()).data() << std::endl;
Sleep(1000);
std::cout << easy::datetime::localtime_string(easy::datetime::current_time_point()).data() << std::endl;

std::cout << "----------------------------------------------------------------------" << std::endl;

auto _00 = easy::datetime::from_timestamp<std::chrono::seconds>(0);                 // 1970-01-01 00:00:00
auto _01 = easy::datetime::from_timestamp<std::chrono::seconds>(57600);             // +16 hours
auto _02 = easy::datetime::from_timestamp<std::chrono::seconds>(86400);             // +24 hours
auto _03 = easy::datetime::from_timestamp<std::chrono::seconds>(90000);             // +25 hours
auto _04 = easy::datetime::from_timestamp<std::chrono::seconds>(144000);            // +1 days 16 hours
auto _05 = easy::datetime::from_timestamp<std::chrono::seconds>(172800);            // +2 days
auto _06 = easy::datetime::from_timestamp<std::chrono::seconds>(28800 - 28800);     // 0
auto _07 = easy::datetime::from_timestamp<std::chrono::seconds>(57600 - 28800);     // +16 hours - 8 hours
auto _08 = easy::datetime::from_timestamp<std::chrono::seconds>(86400 - 28800);     // +24 hours - 8 hours
auto _09 = easy::datetime::from_timestamp<std::chrono::seconds>(90000 - 28800);     // +25 hours - 8 hours
auto _10 = easy::datetime::from_timestamp<std::chrono::seconds>(144000 - 28800);    // +40 hours - 8 hours
auto _11 = easy::datetime::from_timestamp<std::chrono::seconds>(172800 - 28800);    // +48 hours - 8 hours

std::cout << "local: " << easy::datetime::localtime_string(_00) << " utc: " << easy::datetime::utc_string(_00) << "  // 0" << std::endl;
std::cout << "local: " << easy::datetime::localtime_string(_01) << " utc: " << easy::datetime::utc_string(_01) << "  // +16 hours" << std::endl;
std::cout << "local: " << easy::datetime::localtime_string(_02) << " utc: " << easy::datetime::utc_string(_02) << "  // +24 hours" << std::endl;
std::cout << "local: " << easy::datetime::localtime_string(_03) << " utc: " << easy::datetime::utc_string(_03) << "  // +25 hours" << std::endl;
std::cout << "local: " << easy::datetime::localtime_string(_04) << " utc: " << easy::datetime::utc_string(_04) << "  // +1 days 16 hours" << std::endl;
std::cout << "local: " << easy::datetime::localtime_string(_05) << " utc: " << easy::datetime::utc_string(_05) << "  // +2 days" << std::endl;
std::cout << "local: " << easy::datetime::localtime_string(_06) << " utc: " << easy::datetime::utc_string(_06) << "  // 0" << std::endl;
std::cout << "local: " << easy::datetime::localtime_string(_07) << " utc: " << easy::datetime::utc_string(_07) << "  // +16 hours - 8 hours" << std::endl;
std::cout << "local: " << easy::datetime::localtime_string(_08) << " utc: " << easy::datetime::utc_string(_08) << "  // +24 hours - 8 hours" << std::endl;
std::cout << "local: " << easy::datetime::localtime_string(_09) << " utc: " << easy::datetime::utc_string(_09) << "  // +25 hours - 8 hours" << std::endl;
std::cout << "local: " << easy::datetime::localtime_string(_10) << " utc: " << easy::datetime::utc_string(_10) << "  // +40 hours - 8 hours" << std::endl;
std::cout << "local: " << easy::datetime::localtime_string(_11) << " utc: " << easy::datetime::utc_string(_11) << "  // +48 hours - 8 hours" << std::endl;

std::cout << "----------------------------------------------------------------------" << std::endl;
 *
 */
