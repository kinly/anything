#pragma once
#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

using property_type = uint32_t;
using property_value = uint64_t;
using property_value_ref = uint64_t;
using property_value_cref = uint64_t;

struct property {
    property_type  _key;
    property_value _value;

    // todo:
    // std::function<void(const property&, const property_value)> _set;
    // std::function<void(const property&)> _get;

    property& operator += (const property& tar) {
        assert(this->_key == tar._key);
        _value += tar._value;
        return *this;
    }
    property& operator -= (const property& tar) {
        assert(this->_key == tar._key);
        _value > tar._value ? _value -= tar._value : _value = 0;
        return *this;
    }

    property_value_cref value() const {
        return _value;
    }
};

struct tree_node : std::enable_shared_from_this<tree_node> {
    // uint64_t _handle;                                   // 唯一标记
    std::string _handle;                                // debug handle
    std::unordered_map<property_type, property_value> _property;   // 属性列表
    std::vector<std::shared_ptr<tree_node>> _children;  // 子节点
    std::weak_ptr<tree_node> _parent;                   // 父节点

    explicit tree_node(const std::string& handle, std::initializer_list<std::pair<const property_type, property_value>> props)
        : _handle(handle)
        , _property(props) {
    }

    /**
     * \brief 挂载子节点
     * \param child 子节点
     * \return true/false
     */
    bool install(std::shared_ptr<tree_node> child) {
        assert(child->_parent.lock() == nullptr);

        child->_parent = weak_from_this();
        _children.push_back(child);

        child->__property_up();

        return true;
    }

    /**
     * \brief 卸载自己
     * \return true/false
     */
    bool uninstall() {
        auto spt = _parent.lock();
        if (spt == nullptr) {
            return true;
        }
        __property_off();
        return true;
    }

    /**
     * \brief 属性修改：加
     * \param attr 属性
     * \param value 增加值
     */
    void property_add(property_type attr, property_value value) {
        __property_add(attr, value);

        std::shared_ptr<tree_node> spt = _parent.lock();
        if (!spt) {
            return;
        }
        spt->property_add(attr, value);
    }

    /**
     * \brief 属性修改：减
     * \param attr 属性
     * \param value 减去值
     */
    void property_sub(property_type attr, property_value value) {
        __property_sub(attr, value);

        std::shared_ptr<tree_node> spt = _parent.lock();
        if (!spt) {
            return;
        }
        spt->property_sub(attr, value);
    }

private:
    /**
     * \brief 自身修改增量
     * \param attr 属性
     * \param value 值
     */
    void __property_add(property_type attr, property_value value) {
        auto pair_insert = _property.insert({ attr, value });
        if (!pair_insert.second) {
            pair_insert.first->second += value;
        } else {
            pair_insert.first->second = value;
        }
    }

    /**
     * \brief 自身修改减量
     * \param attr 属性
     * \param value 值
     */
    void __property_sub(property_type attr, property_value value) {
        auto pair_insert = _property.insert({ attr, value });
        if (!pair_insert.second) {
            pair_insert.first->second > value ? pair_insert.first->second -= value : pair_insert.first->second = 0;
        } else {
            // assert
        }
    }

    /**
     * \brief 挂载
     */
    void __property_up() {
        std::shared_ptr<tree_node> spt = _parent.lock();
        if (!spt) {
            return;
        }
        for (const auto& one : _property) {
            spt->property_add(one.first, one.second);
        }
    }

    /**
     * \brief 卸载
     */
    void __property_off() {
        std::shared_ptr<tree_node> spt = _parent.lock();
        if (!spt) {
            return;
        }
        for (const auto& one : _property) {
            spt->property_sub(one.first, one.second);
        }
    }
};

struct tree {
    std::shared_ptr<tree_node> _root;        // 根节点

    // 属性赋值
    uint64_t traverse_tree() {
        
    }
};

//         root
//         /  \
//      equip  level
//      / | \
//  slot1 2  3

class player {
    tree _property_tree;

public:
    bool init() {
        // root
        std::shared_ptr<tree_node> root = std::make_shared<tree_node>(tree_node("root", { { 1, 10 }, { 2, 20 } }));
        // equip module
        std::shared_ptr<tree_node> equip = std::make_shared<tree_node>(tree_node("equip", { { 11, 1 } }));
        // slot1 node
        std::shared_ptr<tree_node> slot1 = std::make_shared<tree_node>(tree_node("slot1", { { 11, 10 } }));
        // slot2 node
        std::shared_ptr<tree_node> slot2 = std::make_shared<tree_node>(tree_node("slot2", { { 11, 10 } }));

        root->install(equip);
        equip->install(slot1);
        equip->install(slot2);

        _property_tree._root = root;

        slot1->uninstall();

        return true;
    }
};

/*
player ply;
ply.init();
*/
