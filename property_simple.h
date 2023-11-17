#pragma once
#include <cassert>
#include <cstdint>
#include <functional>
#include <unordered_set>

namespace property {
    class manager;
};

class creature {
private:
    property::manager _property_mgr;

public:
    property::manager& get_property_mgr() {
        return _property_mgr;
    }
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
        using setted_function = std::function(void(creature*, value*, def::value_type));

    private:
        def::key_type _key = 0;
        def::value_type _value = 0;

        setted_function _setted = nullptr;

    private:
        void __refresh(creature* owner) {
            auto old_value = _value;
            _value = meta::calculate();

            if (_setted) {
                _setted(owner, this, old_value);
            }
        }

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
        static value::setted_function __setted_function(def::key_type key) {

            auto DEFAULT_SET = [](creature* owner, value* v, def::value_type old_value) {
                if (owner == nullptr) return;

                static constexpr def::value_type min_value = 0;
                static constexpr def::value_type max_value = 10000;

                static constexpr def::value_type anti_force = 1;

                auto& mgr = owner->get_property_mgr();

                bool anti_monitor = false;

                do {
                    auto calculate_value = v->get_value(true);
                    if (calculate_value < min_value || calculate_value > max_value) {
                        if (anti_monitor) {
                            v->meta::set_percent(0);
                            mgr.set_basic(v->get_key(), anti_force);
                            break;
                        }
                        else {
                            v->meta::set_basic(calculate_value < min_value ? min_value : max_value);
                        }
                    }

                    if (anti_monitor) {
                        if (v->get_value() != v->get_value(true)) {
                            v->set_basic(owner, v->meta::get_basic());
                        }
                        break;
                    }
                    anti_monitor = true;
                } while (true);

                mgr.record_changed(v->get_key());
            };

            auto MAXHP_SET = [DEFAULT_SET](creature* owner, value* v, def::value_type old_value) {
                if (owner == nullptr) return;
                auto& mgr = owner->get_property_mgr();
                const auto current_hp = mgr.get(static_cast<def::key_type>(key::hp));
                if (current_hp > v->get_value() && mgr._borned) {
                    mgr.set_basic(static_cast<def::key_type>(key::hp), v->get_value());
                } else if (current_hp < v->get_value() && mgr._borned) {
                    mgr.add_basic(static_cast<def::key_type>(key::hp), v->get_value() - old_value);
                }
                DEFAULT_SET(owner, v, old_value);
            };

            auto HP_SET = [DEFAULT_SET](creature* owner, value* v, def::value_type old_value) {
                if (owner == nullptr) return;
                auto& mgr = owner->get_property_mgr();
                const auto max_hp = mgr.get(static_cast<def::key_type>(key::max_hp));
                if (v->get_value() > max_hp) {
                    v->meta::set_percent(0);
                    mgr.set_basic(v->get_key(), max_hp);
                }
                DEFAULT_SET(owner, v, old_value);
            };

            auto MAXHPPERCENT_SET = [DEFAULT_SET](creature* owner, value* v, def::value_type old_value) {
                if (owner == nullptr) return;
                auto& mgr = owner->get_property_mgr();
                mgr.set_percent(static_cast<def::key_type>(key::max_hp), v->get_value());
                DEFAULT_SET(owner, v, old_value);
            };

            static std::unordered_map<def::key_type, value::setted_function> setted_functions = {
                { key::max_hp, MAXHP_SET },
                { key::hp, HP_SET },
                { key::max_hp_percent, MAXHPPERCENT_SET },
            };

            auto iter = setted_functions.find(key);
            return iter == setted_functions.end() ? DEFAULT_SET : iter->second;
        }

        auto __key_iterator(def::key_type key, bool inexist_emplace = false) -> decltype(_propertys)::iterator{
            auto iter = _propertys.find(key);
            if (iter == _propertys.end() && inexist_emplace) {
                return _propertys.emplace(key, value(key, __setted_function(key))).first;
            }
            return iter;
        }

        void record_changed(def::key_type key) {
            _last_changed.emplace(key);
        }

    public:
        explicit manager(creature* owner)
            : _owner(owner) {
            
        }

        ~manager() = default;

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
};
