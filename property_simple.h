#pragma once
#include <cassert>
#include <cstdint>
#include <functional>
#include <unordered_set>

namespace property {
    class manager;

    class creature;
};

namespace property {

    namespace def {
        using key_type = uint32_t;
        using value_type = int64_t;
    };

    enum class key : def::key_type {
        max_hp,
        hp,
        max_hp_percent,
    };

    /// @brief meta data: basic & percent
    class meta {
        static constexpr def::value_type denom = 100;

    protected:
        def::value_type _basic = 0;
        def::value_type _percent = 0;

    public:
        meta() = default;
        virtual ~meta() = default;

        def::value_type get_basic() const {
            return _basic;
        }
        void set_basic(def::value_type value) {
            _basic = value;
        }

        def::value_type get_percent() const {
            return _percent;
        }
        void set_percent(def::value_type value) {
            _percent = value;
        }

        def::value_type calculate() const {
            return _basic * (denom + _percent) / denom;
        }
    };

    /// @brief value
    class value final : public meta {
    public:
        using setted_function = std::function<void(creature*, value*, def::value_type)>;

    private:
        def::key_type _key = 0;
        def::value_type _value = 0;

        setted_function _setted = nullptr;

    private:
        void __refresh(creature* owner);

    public:
        explicit value(def::key_type key, setted_function&& setted)
            : _key(key)
            , _value(0)
            , _setted(setted) {
            
        }

        ~value() override = default;

        def::key_type get_key() const {
            return _key;
        }

        def::value_type get_value(bool calculate = false) const {
            return calculate ? meta::calculate() : _value;
        }

        void set_basic(creature* owner, def::value_type value) {
            meta::set_basic(value);
            __refresh(owner);
        }

        void set_percent(creature* owner, def::value_type value) {
            meta::set_percent(value);
            __refresh(owner);
        }
    };

    class manager final {
    public:
        static constexpr def::value_type default_value = 0;

    private:
        creature* _owner = nullptr;

        bool _borned = false;  // 出生标记，用于处理 setted 逻辑

        std::unordered_map<def::key_type, value> _propertys;
        std::unordered_set<def::key_type> _last_changed;

    private:
        static value::setted_function __setted_function(def::key_type key);

        auto __key_iterator(def::key_type key, bool inexist_emplace = false) -> decltype(_propertys)::iterator;

        void record_changed(def::key_type key) {
            _last_changed.emplace(key);
        }

    public:
        explicit manager(creature* owner)
            : _owner(owner) {
            
        }

        ~manager() = default;

        void set_borned() {
            _borned = true;
        }

        def::value_type get(def::key_type key) const {
            const auto iter = _propertys.find(key);
            if (iter == _propertys.end()) return default_value;
            return iter->second.get_value();
        }

        bool set_basic(def::key_type key, def::value_type value) {
            const auto iter = __key_iterator(key, true);
            assert(iter != _propertys.end());

            iter->second.set_basic(_owner, value);
            return true;
        }

        bool add_basic(def::key_type key, def::value_type value) {
            const auto iter = __key_iterator(key, true);
            assert(iter != _propertys.end());

            iter->second.set_basic(_owner, iter->second.get_basic() + value);
            return true;
        }

        bool sub_basic(def::key_type key, def::value_type value) {
            const auto iter = __key_iterator(key, false);
            if (iter == _propertys.end()) return false;

            iter->second.set_basic(_owner, iter->second.get_basic() + value);
            return true;
        }


        bool set_percent(def::key_type key, def::value_type value) {
            const auto iter = __key_iterator(key, true);
            assert(iter != _propertys.end());

            iter->second.set_percent(_owner, value);
            return true;
        }

        bool add_percent(def::key_type key, def::value_type value) {
            const auto iter = __key_iterator(key, true);
            assert(iter != _propertys.end());

            iter->second.set_percent(_owner, iter->second.get_percent() + value);
            return true;
        }

        bool sub_percent(def::key_type key, def::value_type value) {
            const auto iter = __key_iterator(key, false);
            if (iter == _propertys.end()) return false;

            iter->second.set_percent(_owner, iter->second.get_percent() - value);
            return true;
        }

        void foreach(std::function<void(def::key_type, def::value_type)>&& doing) const {
            if (doing == nullptr) return;
            for (const auto& one : _propertys) {
                doing(one.second.get_key(), one.second.get_value());
            }
        }

        auto get_changed(bool force_all = false) -> decltype(_last_changed) {
            if (force_all) {
                foreach([this](def::key_type key, def::value_type) {
                    record_changed(key);
                });
            }
            return std::move(_last_changed);
        }

        template<class container_tt, class value_tt>
        static bool anyone(const container_tt& source, std::initializer_list<value_tt>&& targets) {
            for (const auto& one : targets) {
                if (source.count(one) > 0) return true;
            }
            return false;
        }
    };

    class creature {
    private:
        property::manager _property_mgr;

    public:
        creature();

        property::manager& get_property_mgr();
    };
};
