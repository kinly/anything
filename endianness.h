#pragma once
#include <cstdint>

// little-endian or big-endian
namespace endianness {

    static bool is_big_endian() noexcept {
        static int32_t test_number = 1;
        return *reinterpret_cast<int8_t*>(&test_number) != 1;
    }

    static bool is_little_endian() noexcept {
        return !is_big_endian();
    }

    static bool is_be() noexcept {
        return is_big_endian();
    }

    static bool is_le() noexcept {
        return is_little_endian();
    }

    //////////////////////////////////////////////////////////////////////////
    /// read

    template<class tt, std::enable_if_t<std::is_same_v<uint8_t, tt> || std::is_same_v<int8_t, tt>, bool> = true>
    static tt be_read(const void* buffer, tt& value) noexcept {
        reinterpret_cast<uint8_t*>(&value)[0] = static_cast<const uint8_t*>(buffer)[0];
        return value;
    }

    template<class tt, std::enable_if_t<std::is_same_v<uint16_t, tt> || std::is_same_v<int16_t, tt>, bool> = true>
    static tt be_read(const void* buffer, tt& value) noexcept {
        if (is_be()) {
            reinterpret_cast<uint8_t*>(&value)[0] = static_cast<const uint8_t*>(buffer)[0];
            reinterpret_cast<uint8_t*>(&value)[1] = static_cast<const uint8_t*>(buffer)[1];
        } else {
            reinterpret_cast<uint8_t*>(&value)[0] = static_cast<const uint8_t*>(buffer)[1];
            reinterpret_cast<uint8_t*>(&value)[1] = static_cast<const uint8_t*>(buffer)[0];
        }
        return value;
    }

    template<class tt, std::enable_if_t<std::is_same_v<uint32_t, tt> || std::is_same_v<int32_t, tt>, bool> = true>
    static tt be_read(const void* buffer, tt& value) noexcept {
        if (is_be()) {
            reinterpret_cast<uint8_t*>(&value)[0] = static_cast<const uint8_t*>(buffer)[0];
            reinterpret_cast<uint8_t*>(&value)[1] = static_cast<const uint8_t*>(buffer)[1];
            reinterpret_cast<uint8_t*>(&value)[2] = static_cast<const uint8_t*>(buffer)[2];
            reinterpret_cast<uint8_t*>(&value)[3] = static_cast<const uint8_t*>(buffer)[3];
        } else {
            reinterpret_cast<uint8_t*>(&value)[0] = static_cast<const uint8_t*>(buffer)[3];
            reinterpret_cast<uint8_t*>(&value)[1] = static_cast<const uint8_t*>(buffer)[2];
            reinterpret_cast<uint8_t*>(&value)[2] = static_cast<const uint8_t*>(buffer)[1];
            reinterpret_cast<uint8_t*>(&value)[3] = static_cast<const uint8_t*>(buffer)[0];
        }
        return value;
    }

    template<class tt, std::enable_if_t<std::is_same_v<uint64_t, tt> || std::is_same_v<int64_t, tt>, bool> = true>
    static tt be_read(const void* buffer, tt& value) noexcept {
        if (is_be()) {
            reinterpret_cast<uint8_t*>(&value)[0] = static_cast<const uint8_t*>(buffer)[0];
            reinterpret_cast<uint8_t*>(&value)[1] = static_cast<const uint8_t*>(buffer)[1];
            reinterpret_cast<uint8_t*>(&value)[2] = static_cast<const uint8_t*>(buffer)[2];
            reinterpret_cast<uint8_t*>(&value)[3] = static_cast<const uint8_t*>(buffer)[3];
            reinterpret_cast<uint8_t*>(&value)[4] = static_cast<const uint8_t*>(buffer)[4];
            reinterpret_cast<uint8_t*>(&value)[5] = static_cast<const uint8_t*>(buffer)[5];
            reinterpret_cast<uint8_t*>(&value)[6] = static_cast<const uint8_t*>(buffer)[6];
            reinterpret_cast<uint8_t*>(&value)[7] = static_cast<const uint8_t*>(buffer)[7];
        } else {
            reinterpret_cast<uint8_t*>(&value)[0] = static_cast<const uint8_t*>(buffer)[7];
            reinterpret_cast<uint8_t*>(&value)[1] = static_cast<const uint8_t*>(buffer)[6];
            reinterpret_cast<uint8_t*>(&value)[2] = static_cast<const uint8_t*>(buffer)[5];
            reinterpret_cast<uint8_t*>(&value)[3] = static_cast<const uint8_t*>(buffer)[4];
            reinterpret_cast<uint8_t*>(&value)[4] = static_cast<const uint8_t*>(buffer)[3];
            reinterpret_cast<uint8_t*>(&value)[5] = static_cast<const uint8_t*>(buffer)[2];
            reinterpret_cast<uint8_t*>(&value)[6] = static_cast<const uint8_t*>(buffer)[1];
            reinterpret_cast<uint8_t*>(&value)[7] = static_cast<const uint8_t*>(buffer)[0];
        }
        return value;
    }

    template<class tt, std::enable_if_t<std::is_same_v<uint8_t, tt> || std::is_same_v<int8_t, tt>, bool> = true>
    static tt le_read(const void* buffer, tt& value) noexcept {
        reinterpret_cast<uint8_t*>(&value)[0] = static_cast<const uint8_t*>(buffer)[0];
        return value;
    }

    template<class tt, std::enable_if_t<std::is_same_v<uint16_t, tt> || std::is_same_v<int16_t, tt>, bool> = true>
    static tt le_read(const void* buffer, tt& value) noexcept {
        if (is_le()) {
            reinterpret_cast<uint8_t*>(&value)[0] = static_cast<const uint8_t*>(buffer)[0];
            reinterpret_cast<uint8_t*>(&value)[1] = static_cast<const uint8_t*>(buffer)[1];
        } else {
            reinterpret_cast<uint8_t*>(&value)[0] = static_cast<const uint8_t*>(buffer)[1];
            reinterpret_cast<uint8_t*>(&value)[1] = static_cast<const uint8_t*>(buffer)[0];
        }
        return value;
    }

    template<class tt, std::enable_if_t<std::is_same_v<uint32_t, tt> || std::is_same_v<int32_t, tt>, bool> = true>
    static tt le_read(const void* buffer, tt& value) noexcept {
        if (is_le()) {
            reinterpret_cast<uint8_t*>(&value)[0] = static_cast<const uint8_t*>(buffer)[0];
            reinterpret_cast<uint8_t*>(&value)[1] = static_cast<const uint8_t*>(buffer)[1];
            reinterpret_cast<uint8_t*>(&value)[2] = static_cast<const uint8_t*>(buffer)[2];
            reinterpret_cast<uint8_t*>(&value)[3] = static_cast<const uint8_t*>(buffer)[3];
        } else {
            reinterpret_cast<uint8_t*>(&value)[0] = static_cast<const uint8_t*>(buffer)[3];
            reinterpret_cast<uint8_t*>(&value)[1] = static_cast<const uint8_t*>(buffer)[2];
            reinterpret_cast<uint8_t*>(&value)[2] = static_cast<const uint8_t*>(buffer)[1];
            reinterpret_cast<uint8_t*>(&value)[3] = static_cast<const uint8_t*>(buffer)[0];
        }
        return value;
    }

    template<class tt, std::enable_if_t<std::is_same_v<uint64_t, tt> || std::is_same_v<int64_t, tt>, bool> = true>
    static tt le_read(const void* buffer, tt& value) noexcept {
        if (is_le()) {
            reinterpret_cast<uint8_t*>(&value)[0] = static_cast<const uint8_t*>(buffer)[0];
            reinterpret_cast<uint8_t*>(&value)[1] = static_cast<const uint8_t*>(buffer)[1];
            reinterpret_cast<uint8_t*>(&value)[2] = static_cast<const uint8_t*>(buffer)[2];
            reinterpret_cast<uint8_t*>(&value)[3] = static_cast<const uint8_t*>(buffer)[3];
            reinterpret_cast<uint8_t*>(&value)[4] = static_cast<const uint8_t*>(buffer)[4];
            reinterpret_cast<uint8_t*>(&value)[5] = static_cast<const uint8_t*>(buffer)[5];
            reinterpret_cast<uint8_t*>(&value)[6] = static_cast<const uint8_t*>(buffer)[6];
            reinterpret_cast<uint8_t*>(&value)[7] = static_cast<const uint8_t*>(buffer)[7];
        } else {
            reinterpret_cast<uint8_t*>(&value)[0] = static_cast<const uint8_t*>(buffer)[7];
            reinterpret_cast<uint8_t*>(&value)[1] = static_cast<const uint8_t*>(buffer)[6];
            reinterpret_cast<uint8_t*>(&value)[2] = static_cast<const uint8_t*>(buffer)[5];
            reinterpret_cast<uint8_t*>(&value)[3] = static_cast<const uint8_t*>(buffer)[4];
            reinterpret_cast<uint8_t*>(&value)[4] = static_cast<const uint8_t*>(buffer)[3];
            reinterpret_cast<uint8_t*>(&value)[5] = static_cast<const uint8_t*>(buffer)[2];
            reinterpret_cast<uint8_t*>(&value)[6] = static_cast<const uint8_t*>(buffer)[1];
            reinterpret_cast<uint8_t*>(&value)[7] = static_cast<const uint8_t*>(buffer)[0];
        }
        return value;
    }

    //////////////////////////////////////////////////////////////////////////
    /// write

    template<class tt, std::enable_if_t<std::is_same_v<uint8_t, tt> || std::is_same_v<int8_t, tt>, bool> = true>
    static void be_write(tt value, void* buffer) noexcept {
        static_cast<uint8_t*>(buffer)[0] = reinterpret_cast<const uint8_t*>(&value)[0];
    }

    template<class tt, std::enable_if_t<std::is_same_v<uint16_t, tt> || std::is_same_v<int16_t, tt>, bool> = true>
    static void be_write(tt value, void* buffer) noexcept {
        if (is_be()) {
            static_cast<uint8_t*>(buffer)[0] = reinterpret_cast<const uint8_t*>(&value)[0];
            static_cast<uint8_t*>(buffer)[1] = reinterpret_cast<const uint8_t*>(&value)[1];
        } else {
            static_cast<uint8_t*>(buffer)[0] = reinterpret_cast<const uint8_t*>(&value)[1];
            static_cast<uint8_t*>(buffer)[1] = reinterpret_cast<const uint8_t*>(&value)[0];
        }
    }

    template<class tt, std::enable_if_t<std::is_same_v<uint32_t, tt> || std::is_same_v<int32_t, tt>, bool> = true>
    static void be_write(tt value, void* buffer) noexcept {
        if (is_be()) {
            static_cast<uint8_t*>(buffer)[0] = reinterpret_cast<const uint8_t*>(&value)[0];
            static_cast<uint8_t*>(buffer)[1] = reinterpret_cast<const uint8_t*>(&value)[1];
            static_cast<uint8_t*>(buffer)[2] = reinterpret_cast<const uint8_t*>(&value)[2];
            static_cast<uint8_t*>(buffer)[3] = reinterpret_cast<const uint8_t*>(&value)[3];
        } else {
            static_cast<uint8_t*>(buffer)[0] = reinterpret_cast<const uint8_t*>(&value)[3];
            static_cast<uint8_t*>(buffer)[1] = reinterpret_cast<const uint8_t*>(&value)[2];
            static_cast<uint8_t*>(buffer)[2] = reinterpret_cast<const uint8_t*>(&value)[1];
            static_cast<uint8_t*>(buffer)[3] = reinterpret_cast<const uint8_t*>(&value)[0];
        }
    }

    template<class tt, std::enable_if_t<std::is_same_v<uint64_t, tt> || std::is_same_v<int64_t, tt>, bool> = true>
    static void be_write(tt value, void* buffer) noexcept {
        if (is_be()) {
            static_cast<uint8_t*>(buffer)[0] = reinterpret_cast<const uint8_t*>(&value)[0];
            static_cast<uint8_t*>(buffer)[1] = reinterpret_cast<const uint8_t*>(&value)[1];
            static_cast<uint8_t*>(buffer)[2] = reinterpret_cast<const uint8_t*>(&value)[2];
            static_cast<uint8_t*>(buffer)[3] = reinterpret_cast<const uint8_t*>(&value)[3];
            static_cast<uint8_t*>(buffer)[4] = reinterpret_cast<const uint8_t*>(&value)[4];
            static_cast<uint8_t*>(buffer)[5] = reinterpret_cast<const uint8_t*>(&value)[5];
            static_cast<uint8_t*>(buffer)[6] = reinterpret_cast<const uint8_t*>(&value)[6];
            static_cast<uint8_t*>(buffer)[7] = reinterpret_cast<const uint8_t*>(&value)[7];
        } else {
            static_cast<uint8_t*>(buffer)[0] = reinterpret_cast<const uint8_t*>(&value)[7];
            static_cast<uint8_t*>(buffer)[1] = reinterpret_cast<const uint8_t*>(&value)[6];
            static_cast<uint8_t*>(buffer)[2] = reinterpret_cast<const uint8_t*>(&value)[5];
            static_cast<uint8_t*>(buffer)[3] = reinterpret_cast<const uint8_t*>(&value)[4];
            static_cast<uint8_t*>(buffer)[4] = reinterpret_cast<const uint8_t*>(&value)[3];
            static_cast<uint8_t*>(buffer)[5] = reinterpret_cast<const uint8_t*>(&value)[2];
            static_cast<uint8_t*>(buffer)[6] = reinterpret_cast<const uint8_t*>(&value)[1];
            static_cast<uint8_t*>(buffer)[7] = reinterpret_cast<const uint8_t*>(&value)[0];
        }
    }

    template<class tt, std::enable_if_t<std::is_same_v<uint8_t, tt> || std::is_same_v<int8_t, tt>, bool> = true>
    static void le_write(tt value, void* buffer) noexcept {
        static_cast<uint8_t*>(buffer)[0] = reinterpret_cast<const uint8_t*>(&value)[0];
    }

    template<class tt, std::enable_if_t<std::is_same_v<uint16_t, tt> || std::is_same_v<int16_t, tt>, bool> = true>
    static void le_write(tt value, void* buffer) noexcept {
        if (is_le()) {
            static_cast<uint8_t*>(buffer)[0] = reinterpret_cast<const uint8_t*>(&value)[0];
            static_cast<uint8_t*>(buffer)[1] = reinterpret_cast<const uint8_t*>(&value)[1];
        } else {
            static_cast<uint8_t*>(buffer)[0] = reinterpret_cast<const uint8_t*>(&value)[1];
            static_cast<uint8_t*>(buffer)[1] = reinterpret_cast<const uint8_t*>(&value)[0];
        }
    }

    template<class tt, std::enable_if_t<std::is_same_v<uint32_t, tt> || std::is_same_v<int32_t, tt>, bool> = true>
    static void le_write(tt value, void* buffer) noexcept {
        if (is_le()) {
            static_cast<uint8_t*>(buffer)[0] = reinterpret_cast<const uint8_t*>(&value)[0];
            static_cast<uint8_t*>(buffer)[1] = reinterpret_cast<const uint8_t*>(&value)[1];
            static_cast<uint8_t*>(buffer)[2] = reinterpret_cast<const uint8_t*>(&value)[2];
            static_cast<uint8_t*>(buffer)[3] = reinterpret_cast<const uint8_t*>(&value)[3];
        } else {
            static_cast<uint8_t*>(buffer)[0] = reinterpret_cast<const uint8_t*>(&value)[3];
            static_cast<uint8_t*>(buffer)[1] = reinterpret_cast<const uint8_t*>(&value)[2];
            static_cast<uint8_t*>(buffer)[2] = reinterpret_cast<const uint8_t*>(&value)[1];
            static_cast<uint8_t*>(buffer)[3] = reinterpret_cast<const uint8_t*>(&value)[0];
        }
    }

    template<class tt, std::enable_if_t<std::is_same_v<uint64_t, tt> || std::is_same_v<int64_t, tt>, bool> = true>
    static void le_write(tt value, void* buffer) noexcept {
        if (is_le()) {
            static_cast<uint8_t*>(buffer)[0] = reinterpret_cast<const uint8_t*>(&value)[0];
            static_cast<uint8_t*>(buffer)[1] = reinterpret_cast<const uint8_t*>(&value)[1];
            static_cast<uint8_t*>(buffer)[2] = reinterpret_cast<const uint8_t*>(&value)[2];
            static_cast<uint8_t*>(buffer)[3] = reinterpret_cast<const uint8_t*>(&value)[3];
            static_cast<uint8_t*>(buffer)[4] = reinterpret_cast<const uint8_t*>(&value)[4];
            static_cast<uint8_t*>(buffer)[5] = reinterpret_cast<const uint8_t*>(&value)[5];
            static_cast<uint8_t*>(buffer)[6] = reinterpret_cast<const uint8_t*>(&value)[6];
            static_cast<uint8_t*>(buffer)[7] = reinterpret_cast<const uint8_t*>(&value)[7];
        } else {
            static_cast<uint8_t*>(buffer)[0] = reinterpret_cast<const uint8_t*>(&value)[7];
            static_cast<uint8_t*>(buffer)[1] = reinterpret_cast<const uint8_t*>(&value)[6];
            static_cast<uint8_t*>(buffer)[2] = reinterpret_cast<const uint8_t*>(&value)[5];
            static_cast<uint8_t*>(buffer)[3] = reinterpret_cast<const uint8_t*>(&value)[4];
            static_cast<uint8_t*>(buffer)[4] = reinterpret_cast<const uint8_t*>(&value)[3];
            static_cast<uint8_t*>(buffer)[5] = reinterpret_cast<const uint8_t*>(&value)[2];
            static_cast<uint8_t*>(buffer)[6] = reinterpret_cast<const uint8_t*>(&value)[1];
            static_cast<uint8_t*>(buffer)[7] = reinterpret_cast<const uint8_t*>(&value)[0];
        }
    }

}; // end namespace endianness

