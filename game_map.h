#pragma once
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <unordered_set>
#include <utility>

class entity;

class entity {
public:
    uint64_t _id = 0;
    bool _barrier = true;
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

    struct cell_flag {
        union {
            int _flag = 0;
            struct {
                bool _block   : 1;    // 物理阻挡
                bool _barrier : 1;    // 占用阻挡
            };
        };

        cell_flag() = default;

        bool is_block() const {
            return _block;
        }

        bool is_barrier() const {
            return _barrier;
        }

        bool is_free(bool absolution = true) const {
            return !is_block() && ((absolution && !is_barrier()) || true);
        }

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
                    cpt.cx / _config.area_size.x,
                    cpt.cy / _config.area_size.y
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

            void exchange_cell(entity* ptr, const cell_point& from, const cell_point& to) {
                if (from == to) return;
                if (ptr->_barrier) {
                    if (cell_ok(from)) {
                        _cells[cell2index(from)]._barrier = false;
                        _event.impl()->exit_cell(ptr, from);
                    }
                    if (cell_ok(to)) {
                        _cells[cell2index(to)]._barrier = true;
                        _event.impl()->enter_cell(ptr, to);
                    }
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
    };
}; // end namespace map
