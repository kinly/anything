#pragma once
#include <any>
#include <string>
#include <unordered_map>

// 实现了一套用于管理entity和character对象属性的类attribute_mgr
// attribute_mgr可以管理entity和character两种类型的属性，同时支持重载，当检索不到个对象的对应属性时，会反问其父类对象以获取对应属性
// entity_base作为entity的基类，其中包含了entity的属性管理器，entity作为其派生类，再额外增加了基于entity的属性管理器
// character作为entity的派生类，在entity的基础上增加了基于entity的属性管理器，并初始化了一些对象及其属性的值

namespace attr_link {

    enum attribute_type {  // 定义枚举类型，包含四个可选状态（undefined，integer，float_，string_）
        undefined,
        integer_,
        float_,
        string_
    };

    struct attribute {  // 自定义类型，包含属性名称，属性ID，属性类型，属性值
        std::string name;
        int index{};
        attribute_type type{ undefined };
        std::any value;
    };

    // 自定义模板类型，带有模板参数 base_class
    template <typename base_class>
    struct attribute_manager {  // 属性管理器
        std::unordered_map<int, attribute> attributes;  // 属性ID 和 属性对象 的键值对

        // 模板函数，返回属性ID是否存在
        template <typename ty>
        bool is_index_valid(ty& object, int idx) {
            if (attributes.count(idx) > 0)  // 判断属性ID是否存在于 attributes 键值对中
                return true;

            // 当前属性ID不在 attributes 键值对中，向前寻找 base_class（非本身） 的 is_index_valid 函数
            return object.base_class::_attribute_manager.is_index_valid(object, idx);
        }

        attribute_manager() {}  // 默认构造函数
    };

    struct empty_base {};  // 自定义类型，用于作为空基类（不带任何属性）

    // 当模板参数 base_class 为 empty_base 时，即派生类不再派生自其他派生类
    template <>
    struct attribute_manager<empty_base> {
        template<typename ty>
        bool is_index_valid(ty& object, int) {  // 当前空基类不需要判断任何属性ID的合法性
            return false;
        }
    };

    struct entity_base {
        attribute_manager<empty_base> _attribute_manager;  // 内部嵌套一个用于管理空基类的属性管理器
    };

    struct entity : public entity_base {
        attribute_manager<entity_base> _attribute_manager;  // 本身的属性管理器，派生于entity_base内的空基类属性管理器

        entity() : entity_base() {
            _attribute_manager.attributes.insert(std::make_pair(1, attribute({ "uuid", 1, attribute_type::integer_, 0 })));
            _attribute_manager.attributes.insert(std::make_pair(2, attribute({ "meta", 2, attribute_type::string_, "" })));
        }
    };

    struct character : public entity {
        attribute_manager<entity> _attribute_manager;  // 本身的属性管理器，派生于继承自entity的属性管理器

        character() : entity() {
            _attribute_manager.attributes.insert(std::make_pair(11, attribute({ "pos_x", 11, attribute_type::integer_, 0 })));
            _attribute_manager.attributes.insert(std::make_pair(12, attribute({ "pos_y", 12, attribute_type::integer_, 0 })));
        }
    };
}
/*
 *
    {
        attr_link::character* ptr = new attr_link::character;
        assert(ptr->_attribute_manager.is_index_valid(*ptr, 11));
        assert(ptr->_attribute_manager.is_index_valid(*ptr, 1));
        attr_link::entity* ptr_entity = ptr;
        auto false_ = ptr_entity->_attribute_manager.is_index_valid(*ptr_entity, 11);
        // break-point
        int i = 0;
        i += 1;
    }
 *
 */
