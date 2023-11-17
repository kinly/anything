#include "property.h"


property::creature::creature() : _property_mgr(this) {
}

property::manager& property::creature::get_property_mgr() {
    return _property_mgr;
}

void property::value::__refresh(creature* owner) {
    auto old_value = _value;
    _value = meta::calculate();

    if (_setted) {
        _setted(owner, this, old_value);
    }
}

property::value::setted_function property::manager::__setted_function(def::key_type key) {

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
        { static_cast<def::key_type>(key::max_hp), MAXHP_SET },
        { static_cast<def::key_type>(key::hp), HP_SET },
        { static_cast<def::key_type>(key::max_hp_percent), MAXHPPERCENT_SET },
    };

    auto iter = setted_functions.find(key);
    return iter == setted_functions.end() ? DEFAULT_SET : iter->second;
}

auto property::manager::__key_iterator(def::key_type key, bool inexist_emplace) -> decltype(_propertys)::iterator {
    auto iter = _propertys.find(key);
    if (iter == _propertys.end() && inexist_emplace) {
        return _propertys.emplace(key, value(key, __setted_function(key))).first;
    }
    return iter;
}

void property_simple_test() {
    property::creature* owner = new property::creature();

    owner->get_property_mgr().set_basic(static_cast<property::def::key_type>(property::key::max_hp), 100);
    owner->get_property_mgr().set_basic(static_cast<property::def::key_type>(property::key::hp), 100);

    owner->get_property_mgr().set_borned();

    owner->get_property_mgr().add_basic(static_cast<property::def::key_type>(property::key::max_hp), 20);

    std::cout << owner->get_property_mgr().get(static_cast<property::def::key_type>(property::key::hp)) << std::endl;

    owner->get_property_mgr().add_basic(static_cast<property::def::key_type>(property::key::max_hp_percent), 100);

    std::cout << owner->get_property_mgr().get(static_cast<property::def::key_type>(property::key::hp)) << std::endl;
}
