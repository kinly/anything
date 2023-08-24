#pragma once
#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace base {
    class non_lock final {
    public:
        void lock() {}
        void unlock() {}
    };

    /**
     * \brief handle bits 64
     * |--- cell --- | --- chunk --- | --- round --- | --- crc --- | --- type --- |
     * |========== data =============| ========== checker =========| === type === |
     * |======== 16777215 ===========| =========== 4096 ===========| === 256  === |
     * 16777215's handle max (each type), repeated distance (64~4096), 256's type max
     */
    enum class handle_bits_default : unsigned {
        cell = 12,     // data:    cell  bits
        chunk = 12,    // data:    chunk bits
        rnd = 6,       // checker: rnd   bits
        crc = 6,       // checker: crc   bits
        type = 8,      // type:    type  bits
    };

    /**
     * \brief handle 数据类型
     */
    using handle_type_default = uint64_t;

    class handle_value {
    public:
        using type = handle_type_default;
        using bits = handle_bits_default;

        static constexpr type invalid_handle = (1 << (static_cast<unsigned>(bits::cell)
            + static_cast<unsigned>(bits::chunk)
            + static_cast<unsigned>(bits::rnd)
            + static_cast<unsigned>(bits::crc)
            + static_cast<unsigned>(bits::type)
            )) - 1;

        union {
            type _handle = 0;

            struct {
                type _cell : static_cast<unsigned>(bits::cell);
                type _chunk : static_cast<unsigned>(bits::chunk);
                type _rnd : static_cast<unsigned>(bits::rnd);
                type _crc : static_cast<unsigned>(bits::crc);
                type _type : static_cast<unsigned>(bits::type);
            };
        };

        handle_value()
            : _handle(invalid_handle) {
            
        }

        handle_value(type source)
            : _handle(source) {
            
        }

        virtual ~handle_value() = default;

        bool operator == (type source) const {
            return _handle == source;
        }

        bool operator == (const handle_value& source) const {
            return _handle == source._handle;
        }
    }; // end class handle_value

    template<typename _Child_Type, typename _Handle_Value = handle_value>
    class entity_handle : public _Handle_Value {
    public:
        using entity_type = uint16_t;
        using entity_handle_type = typename _Handle_Value::type;

        entity_handle() = default;
        virtual ~entity_handle() = default;

        /**
         * \brief 句柄
         * \return 句柄
         */
        entity_handle_type handle() const {
            return this->_handle;
        }

        /**
         * \brief 类型
         * \return 类型
         */
        virtual entity_type get_type() = 0;

        template<typename _Instance>
        bool instance_of() {
            return dynamic_cast<_Instance*>(this) != nullptr;
        }

        /**
         * \brief 创建
         * \param ptr 对象
         * \return 句柄
         */
        static entity_handle_type new_handle(entity_handle* ptr);

        /**
         * \brief 销毁
         * \param ptr 对象
         */
        static void del_handle(entity_handle* ptr);

        /**
         * \brief 转换
         * \param handle_value 句柄
         * \return 对象
         */
        static _Child_Type* exchange(entity_handle_type handle_value);

        /**
         * \brief 创建
         * \param ptr 对象
         * \return 句柄
         */
        static entity_handle_type new_handle_mt(entity_handle* ptr);

        /**
         * \brief 销毁
         * \param ptr 对象
         */
        static void del_handle_mt(entity_handle* ptr);

        /**
         * \brief 转换
         * \param handle_value 句柄
         * \return 对象
         */
        static _Child_Type* exchange_mt(entity_handle_type handle_value);
    }; // end class entity_handle

    template<typename _Type, typename _Handle_Value, typename _Lock = non_lock>
    class entity_pool final {
        using type = _Type;
        using entity_handle_t = entity_handle<_Type, _Handle_Value>;
        using entity_point = entity_handle_t*;
        using lock_type = _Lock;

        static constexpr std::size_t max_cell_size = (1 << static_cast<unsigned>(_Handle_Value::bits::cell));
        static constexpr std::size_t max_chunk_size = (1 << static_cast<unsigned>(_Handle_Value::bits::chunk));
        static constexpr std::size_t max_round = ((1 << static_cast<unsigned>(_Handle_Value::bits::rnd)) - 1);
        static constexpr std::size_t max_crc = ((1 << static_cast<unsigned>(_Handle_Value::bits::crc)) - 1);
        static constexpr std::size_t max_type = ((1 << static_cast<unsigned>(_Handle_Value::bits::type)) - 1);

        using cell_array = std::array<entity_point, max_cell_size>;

        struct chunk {
            typename entity_handle_t::type _next = 0;   // 下一个空点，-1 表示满
            typename entity_handle_t::type _alloc = 0;  // 已分配的数量
            typename entity_handle_t::type _crc = 0;    // crc flag

            cell_array _cells = { nullptr };        // cells

            void release() {
                for (auto one : _cells) {
                    delete one;
                    one = nullptr;
                }
            }
        }; // end struct chunk

    private:
        mutable lock_type _lock;
        std::vector<chunk> _chunks;                       // chunks
        typename entity_handle_t::type _rnd = 0;          // round flag
        typename entity_handle_t::type _last_chunk = 0;   // 可用的 chunk

        std::atomic<std::size_t> _total;                  // 总数
        std::unordered_map<typename entity_handle_t::entity_type, std::size_t> _type2count;

    private:
        chunk& new_chunk() {
            chunk chunk_;
            chunk_._next = 0;
            chunk_._alloc = 0;
            chunk_._crc = 1;
            _chunks.emplace_back(std::move(chunk_));
            return _chunks.back();
        }

    public:
        static entity_pool<_Type, _Handle_Value>& instance() {
            static entity_pool<_Type, _Handle_Value> inst;
            return inst;
        }

        explicit entity_pool(uint16_t init_chunk = 8) {
            assert(init_chunk < max_chunk_size);

            std::lock_guard<lock_type> lock(_lock);

            while (init_chunk--) {
                new_chunk();
            }

            _rnd = 1;
            _last_chunk = 0;
            _total = 0;
        }

        ~entity_pool() {
            std::lock_guard<lock_type> lock(_lock);

            for (auto& one : _chunks) {
                one.release();
            }
            std::vector<chunk>().swap(_chunks);

            _rnd = 0;
            _last_chunk = 0;
            _total = 0;
            _type2count.clear();
        }

        std::size_t total() const {
            return _total;
        }

        void debug_msg(std::is_function<void(std::size_t, std::size_t, std::size_t)>&& debug) {
            if (debug) {
                std::lock_guard<lock_type> lock(_lock);
                debug(max_cell_size, _chunks.size(), _total);
            }
        }

        typename _Handle_Value::type new_handle(entity_point ptr) {
            std::lock_guard<lock_type> lock(_lock);

            chunk* chunk_ = &_chunks.at(_last_chunk);

            while (chunk_->_next >= chunk_->_cells.max_size() || chunk_->_cells.at(chunk_->_next) != nullptr) {
                if (chunk_->_next >= chunk_->_cells.max_size() || ++chunk_->_next >= chunk_->_cells.max_size()) {
                    if (_last_chunk + 1 >= _chunks.size()) {
                        if (_last_chunk + 1 >= max_chunk_size) {
                            throw std::range_error("empty space alloc handle");
                            return entity_handle_t::invalid_handle;
                        }
                        _last_chunk += 1;
                        chunk_ = &new_chunk();
                    } else {
                        _last_chunk += 1;
                        chunk_ = &_chunks.at(_last_chunk);
                        _rnd += 1;

                        if (_rnd > max_round) {
                            _rnd = 0;
                        }
                    }
                }
            }

            ptr->_cell = chunk_->_next;
            ptr->_chunk = _last_chunk;
            ptr->_rnd = (_rnd & max_round);
            ptr->_crc = ((++chunk_->_crc) & max_crc);
            ptr->_type = (ptr->get_type() & max_type);
            chunk_->_cells[ptr->_cell] = ptr;
            chunk_->_alloc += 1;
            chunk_->_next += 1;

            if (chunk_->_crc > max_crc) {
                chunk_->_crc = 0;
            }

            _total += 1;
            _type2count[ptr->_type] += 1;

            return ptr->_handle;
        }

        void del_handle(entity_point ptr) {
            if (ptr == nullptr || ptr != exchange(ptr->_handle)) {
                assert(false);
                return;
            }

            std::lock_guard<lock_type> lock(_lock);

            chunk* chunk_ = &_chunks.at(ptr->_chunk);
            chunk_->_cells[ptr->_cell] = nullptr;
            chunk_->_alloc -= 1;
            chunk_->_next = std::min(chunk_->_next, ptr->_cell);

            _last_chunk = std::min(_last_chunk, ptr->_chunk);
            _total -= 1;
            _type2count[ptr->_type] -= 1;
        }

        type* exchange(typename _Handle_Value::type handle) {
            std::lock_guard<lock_type> lock(_lock);

            _Handle_Value handle_(handle);

            if (handle_._chunk >= _chunks.size()
                || handle_._cell >= _chunks[handle_._chunk]._cells.max_size()) {
                return nullptr;
            }

            auto ptr = dynamic_cast<type*>(_chunks[handle_._chunk]._cells[handle_._cell]);
            return ptr && handle_ == ptr->_handle ? ptr : nullptr;
        }
    }; // end class entity_pool

    /**
     * \brief 新建handle
     * \param ptr 指针
     * \return handle
     */
    template <typename _Child_Type, typename _Handle_Value>
    typename entity_handle<_Child_Type, _Handle_Value>::entity_handle_type entity_handle<_Child_Type, _Handle_Value>::new_handle(entity_handle* ptr) {
        return entity_pool<_Child_Type, _Handle_Value>::instance().new_handle(ptr);
    }

    /**
     * \brief 删除（回收）handle
     * \param ptr 指针
     */
    template <typename _Child_Type, typename _Handle_Value>
    void entity_handle<_Child_Type, _Handle_Value>::del_handle(entity_handle* ptr) {
        entity_pool<_Child_Type, _Handle_Value>::instance().del_handle(ptr);
    }

    /**
     * \brief 转换
     * \param handle_value 句柄值
     * \return 指针
     */
    template <typename _Child_Type, typename _Handle_Value>
    _Child_Type* entity_handle<_Child_Type, _Handle_Value>::exchange(entity_handle_type handle_value) {
        return entity_pool<_Child_Type, _Handle_Value>::instance().exchange(handle_value);
    }

    template <typename _Child_Type, typename _Handle_Value>
    typename entity_handle<_Child_Type, _Handle_Value>::entity_handle_type entity_handle<_Child_Type, _Handle_Value>::new_handle_mt(entity_handle* ptr) {
        return entity_pool<_Child_Type, _Handle_Value, std::mutex>::instance().new_handle(ptr);
    }

    template <typename _Child_Type, typename _Handle_Value>
    void entity_handle<_Child_Type, _Handle_Value>::del_handle_mt(entity_handle* ptr) {
        entity_pool<_Child_Type, _Handle_Value, std::mutex>::instance().del_handle(ptr);
    }

    template <typename _Child_Type, typename _Handle_Value>
    _Child_Type* entity_handle<_Child_Type, _Handle_Value>::exchange_mt(entity_handle_type handle_value) {
        return entity_pool<_Child_Type, _Handle_Value, std::mutex>::instance().exchange(handle_value);
    }

}; // end namespace base


/*
 *
enum class enum_entity_type : uint16_t {
    unknown = 0,

    // basic_entity = 1,
    // scene_entity,

    scene_item,
    // creature,

    player,
    monster,
    npc,
};

 *               basic_entity
 *                    |
 *               scene_entity
 *                    |
 * -----------------------------------------
 *      |                        |
 * scene_item                 creature
 *                               |
 *                  ----------------------------
 *                  |            |             |
 *                player       monster        npc

class basic_entity
    : public base::entity_handle<basic_entity> {
public:
    using uuid = basic_entity::type;

private:
    enum_entity_type _type = enum_entity_type::unknown;

public:
    basic_entity(enum_entity_type type)
        : _type(type) {
        base::entity_handle<basic_entity>::new_handle(this);
    }

    ~basic_entity() override {
        base::entity_handle<basic_entity>::del_handle(this);
    }

    entity_type get_type() final {
        return static_cast<uint16_t>(_type);
    }
};

class scene_entity : public basic_entity {
public:
    scene_entity(enum_entity_type type)
        : basic_entity(type) {

    }

    ~scene_entity() override {
    }
};

class scene_item final : public scene_entity {
public:
    scene_item()
        : scene_entity(enum_entity_type::scene_item) {

    }

    static scene_item* exchange(uuid handle) {
        auto ptr = basic_entity::exchange(handle);
        if (ptr == nullptr) return nullptr;
        return dynamic_cast<scene_item*>(ptr);
    }
};

class creature : public scene_entity {
public:
    creature(enum_entity_type type)
        : scene_entity(type) {

    }

    ~creature() override {
    }
};

class player final : public creature {
public:
    player()
        : creature(enum_entity_type::player) {

    }

    static player* exchange(uuid handle) {
        auto ptr = basic_entity::exchange(handle);
        if (ptr == nullptr) return nullptr;
        return dynamic_cast<player*>(ptr);
    }
};

class monster final : public creature {
public:
    monster()
        : creature(enum_entity_type::monster) {

    }

    static monster* exchange(uuid handle) {
        auto ptr = basic_entity::exchange(handle);
        if (ptr == nullptr) return nullptr;
        return dynamic_cast<monster*>(ptr);
    }
};

class npc final : public creature {
public:
    npc()
        : creature(enum_entity_type::npc) {

    }

    static npc* exchange(uuid handle) {
        auto ptr = basic_entity::exchange(handle);
        if (ptr == nullptr) return nullptr;
        return dynamic_cast<npc*>(ptr);
    }
};

int main(int argc, char* argv[]) {
    {
        auto ptr_player = new player();
        auto ptr_monster = new monster();

        std::cout << ptr_player->handle() << std::endl;
        std::cout << ptr_monster->handle() << std::endl;

        auto same_object = player::exchange(ptr_player->handle());

        std::unordered_map<basic_entity::uuid, basic_entity*> alloced;
        while (true) {
            auto ptr = new player();
            auto result = alloced.insert({ ptr->handle(), ptr });
            if (!result.second) {
                std::cout << "assert" << std::endl;
                return -1;
            }

            if (inlay::random::range(1, 10) >= 5) {
                auto handle = ptr->handle();
                delete ptr;
                ptr = nullptr;
                alloced.erase(handle);
            }
        }
    }
}
*/
