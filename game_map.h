#pragma once
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <unordered_set>
#include <utility>

namespace map {
    struct barrier_mark;
}

class entity;

class entity {
public:
    uint64_t _id = 0;
    map::barrier_mark _barrier_mark;
public:
    entity() = default;
    uint64_t handle() const {
        return _id;
    }

};

namespace map {
    /// 站立单位：cell
    /// 视野单位：area
    /// 横轴：    x
    /// 纵轴：    y

    /**
     * \brief 横纵
     */
    struct point {
        uint32_t x = 0;
        uint32_t y = 0;
    };

    /**
     * \brief struct cell point (world point)
     */
    struct cell_point {
        uint32_t cx = 0;
        uint32_t cy = 0;

        bool operator == (const cell_point& right) const {
            return this->cx == right.cx
                && this->cy == right.cy;
        }
    };
    /**
     * \brief invalid cell point
     */
    static const cell_point invalid_cell = cell_point{ 0x7FFFFFFF, 0x7FFFFFFF };

    /**
     * \brief struct area point
     */
    struct area_point {
        uint32_t ax = 0;
        uint32_t ay = 0;

        bool operator == (const area_point& right) const {
            return this->ax == right.ax
                && this->ay == right.ay;
        }
    };
    /**
     * \brief invalid area point
     */
    static const area_point invalid_area = area_point{ 0x7FFFFFFF, 0x7FFFFFFF };

    /**
     * \brief rect_point
     */
    struct rect_point {
        uint32_t lx = 0;
        uint32_t ly = 0;
        uint32_t rx = 0;
        uint32_t ry = 0;
    };

    /**
     * \brief base config
     */
    struct base_config {
        cell_point cell;    // 地图格子数量
        area_point area;    // 视野块占格子数量
        point eyesight;     // 视野区域半径（3*3视野的话这里是 1, 1）
        point area_size;    // 整张地图多少个area（加载地图后计算得到）
    };

    struct barrier_mark {
        const bool is_mark = false;  // 标记判断是否占用格子

        // test 值和下面 cell_flag 一一对应
        union {
            int flag = 0;
            struct {
                bool test_barrier : 1;    // 是否检查其他占用
                bool test_player : 1;     // 是否检查玩家类型的占用
                bool test_monster : 1;    // 是否检查怪物类型的占用
                bool test_item : 1;       // 是否检查道具类型的占用

                bool test_block : 1;      // 是否检查物理阻挡
            };
        };

        barrier_mark() = default;
        barrier_mark(bool _0, bool _1, bool _2, bool _3, bool _4, bool _5)
            : is_mark(_0)
            , test_barrier(_1)
            , test_player(_2)
            , test_monster(_3)
            , test_item(_4)
            , test_block(_5) {
        }
    };

    static const barrier_mark barrier_mark_none(false, false, false, false, false, true);  // 不占格子，检查物理阻挡（比如用于透明场景对象）
    static const barrier_mark barrier_mark_slack(true, true, false, false, false, true);   // 占格子，松弛的，穿人、穿怪、穿道具
    static const barrier_mark barrier_mark_default(true, true, true, true, false, true);   // 占格子，默认的，不穿人、不穿怪、穿道具
    static const barrier_mark barrier_mark_item(true, true, false, false, true, true);     // 占格子，场景道具的，穿人、穿怪、不穿道具

    // 格子标记
    struct cell_flag {
        union {
            int _flag = 0;
            struct {
                bool _barrier : 1;         // 占用阻挡（如果非下面的特有占用，标记这个通用值）
                bool _player_barrier : 1;  // 特有：玩家占用（如果占用的是玩家，标记这个值）
                bool _monster_barrier : 1; // 特有：怪物占用（如果占用的是怪物，标记这个值）
                bool _item_barrier : 1;    // 特有：道具占用（如果占用的是道具，标记这个值）

                bool _block : 1;           // 物理阻挡
            };
        };

        cell_flag() = default;

        bool is_block() const {
            return _block;
        }
    };

    class event {
    public:
        class interface {
        public:
            interface() = default;
            virtual ~interface() = default;

            virtual void exit_cell(const entity*, const cell_point&) const = 0;
            virtual void enter_cell(const entity*, const cell_point&) const = 0;

            virtual void exit_area(const entity*, const entity*, const area_point&) const = 0;
            virtual void enter_area(const entity*, const entity*, const area_point&) const = 0;
        };
    private:
        std::shared_ptr<interface> _impl;
    public:
        explicit event(std::shared_ptr<interface> ptr)
            : _impl(std::move(ptr)) {

        }

        std::shared_ptr<interface> impl() const {
            return _impl;
        }
    };

    class map {
    public:
        using entity_handle = uint64_t;
        using entity_handles = std::unordered_set<entity_handle>;
        using entity_set = std::unordered_set<entity*>;

        using cell = cell_flag;
    private:
        event _event;
        base_config _config;
        std::vector<cell> _cells;                  // 格子信息
        std::vector<entity_handles> _area_entity;  // 视野格子对象列表
    public:
        explicit map(std::shared_ptr<event::interface> evt)
            : _event(std::move(evt)) {
        }
        bool init(uint32_t cx, uint32_t cy, uint32_t ax, uint32_t ay, uint32_t ex, uint32_t ey) {
            _config.cell.cx = cx;
            _config.cell.cy = cy;
            _config.area.ax = ax;
            _config.area.ay = ay;
            _config.eyesight.x = ex;
            _config.eyesight.y = ey;

            _config.area_size.x = (cx + _config.area.ax - 1) / ax;
            _config.area_size.y = (cy + _config.area.ay - 1) / ay;

            _cells.resize(_config.cell.cx * _config.cell.cy);
            _area_entity.resize(_config.area_size.x * _config.area_size.y);

            return true;
        }

        bool cell_ok(const cell_point& cpt) const {
            return cpt.cx < _config.cell.cx
                && cpt.cy < _config.cell.cy;
        }

        bool area_ok(const area_point& apt) const {
            return apt.ax < _config.area_size.x
                && apt.ay < _config.area_size.y;
        }

        cell_point cell_fixed(uint32_t x, uint32_t y) const {
            return std::move(cell_point{
                std::max(0u, std::min(x, _config.cell.cx)),
                std::max(0u, std::min(y, _config.cell.cy))
                }
            );
        }

        area_point area_fixed(uint32_t x, uint_t y) const {
            return std::move(area_point{
                std::max(0u, std::min(x, _config.area_size.x)),
                std::max(0u, std::min(y, _config.area_size.y))
                }
            );
        }

        rect_point rect_fixed(const rect_point& rpt) const {
            return std::move(rect_point{
                std::max(rpt.lx, 0u),
                std::max(rpt.ly, 0u),
                std::min(std::max(rpt.rx, 0u), _config.area_size.x),
                std::min(std::max(rpt.ry, 0u), _config.area_size.y),
                }
            );
        }

        std::size_t cell2index(const cell_point& cpt) const {
            return _config.cell.cx * cpt.cy + cpt.cx;
        }

        std::size_t area2index(const area_point& apt) const {
            return _config.area_size.x * apt.ay + apt.ax;
        }

        area_point cell2area(const cell_point& cpt) const {
            return area_point{
                cpt.cx / _config.area.ax,
                cpt.cy / _config.area.ay
            };
        }

        std::unordered_set<entity*> entitys(const area_point& apt, const std::function<bool(entity*)>&& filter) const {
            if (!area_ok(apt))
                return { };

            std::unordered_set<entity*> result;
            const auto& handles = _area_entity.at(area2index(apt));
            for (const auto& one : handles) {
                // todo:
            }
            return result;
        }

        std::unordered_set<entity*> entitys(const rect_point& rpt, const std::function<bool(entity*)>&& filter) const {
            std::unordered_set<entity*> result;
            for (uint32_t y = rpt.ly; y <= rpt.ry; ++y) {
                for (uint32_t x = rpt.lx; x <= rpt.rx; ++x) {
                    auto ents = entitys(area_fixed(x, y), filter);
                    result.insert(ents.begin(), ents.end());
                }
            }
            return result;
        }

        void logic_cell_barrier(const cell_point& cpt, entity* ptr, bool mark) {
            // todo: 实际对象
            // auto flag = _cells[cell2index(cpt)];
            // if (ptr->is_player()) {
            //     flag._player_barrier = mark;
            // } else if (ptr->is_monster()) {
            //     
            // }
            // ...
        }

        bool logic_test_barrier(const cell_point& cpt, entity* ptr) {
            if (!cell_ok(cpt)) 
                return true;
            return (ptr->_barrier_mark.flag & _cells[cell2index(cpt)]._flag) > 0;
        }

        void exchange_cell(entity* ptr, const cell_point& from, const cell_point& to) {
            if (from == to) return;

            if (cell_ok(from)) {
                logic_cell_barrier(from, ptr, false);
                _event.impl()->exit_cell(ptr, from);
            }
            if (cell_ok(to)) {
                logic_cell_barrier(from, ptr, true);
                _event.impl()->enter_cell(ptr, to);
            }
        }

        void exchange_area(entity* ptr, const area_point& from, const area_point& to, bool force = false) {
            if (!force && from == to)
                return;

            bool ok_from = area_ok(from);
            bool ok_to = area_ok(to);
            if (!ok_from || !ok_to)
                force = true;

            if (ok_from) {
                _area_entity.at(area2index(from)).erase(ptr->handle());
            }

            std::list<entity*> leaves;
            std::list<entity*> enters;

            // 镜像关系
            for (int32_t y = -_config.eyesight.y; y <= _config.eyesight.y; ++y) {
                for (int32_t x = -_config.eyesight.x; x <= _config.eyesight.x; ++x) {

                    // 两个方向偏移后新点和目标点距离小于半径，用两圆香蕉的方式看待
                    if (!force
                        && std::abs(static_cast<int32_t>(from.ax) + x - static_cast<int32_t>(to.ax)) <= _config.eyesight.x
                        && std::abs(static_cast<int32_t>(from.ay) + y - static_cast<int32_t>(to.ay)) <= _config.eyesight.y) {
                        continue;
                    }
                    if (ok_from) {
                        auto leas = entitys(area_point{ from.ax + x, from.ay + y }, nullptr);
                        std::copy(leas.begin(), leas.end(), std::back_inserter(leaves));
                    }
                    if (ok_to) {
                        auto ents = entitys(area_point{ to.ax - x, to.ay + y }, nullptr);
                        std::copy(ents.begin(), ents.end(), std::back_inserter(enters));
                    }
                }
            }

            if (ok_to) {
                _area_entity.at(area2index(to)).insert(ptr->handle());
            }

            for (const auto one : leaves) {
                _event.impl()->exit_area(ptr, one, from);
            }

            for (const auto one : enters) {
                _event.impl()->enter_area(ptr, one, from);
            }
        }
    };
}; // end namespace map
