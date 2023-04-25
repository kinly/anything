#pragma once
#include <iostream>
#include <memory>

/**
 * crtp
 * 静态多态C++模板编程，派生类作为模板参数传递给基类
 */
namespace crtp {
    // https://en.cppreference.com/w/cpp/language/crtp
#ifndef __cpp_explicit_this_parameter // Traditional syntax
    template <class Derived>
    struct Base {
        void name() {
            (static_cast<Derived*>(this))->impl();
        }
    };
    struct D1 : public Base<D1> {
        void impl() {
            std::puts("D1::impl()");
        }
    };
    struct D2 : public Base<D2> {
        void impl() {
            std::puts("D2::impl()");
        }
    };

    void test() {
        // Base<D1> b1; b1.name(); //undefined behavior
        // Base<D2> b2; b2.name(); //undefined behavior
        D1 d1; d1.name();
        D2 d2; d2.name();
    }

#else // C++23 alternative syntax; https://godbolt.org/z/s1o6qTMnP

    struct Base { void name(this auto&& self) { self.impl(); } };
    struct D1 : public Base { void impl() { std::puts("D1::impl()"); } };
    struct D2 : public Base { void impl() { std::puts("D2::impl()"); } };

    void test() {
        D1 d1; d1.name();
        D2 d2; d2.name();
    }

#endif
}; // end namespace crtp

/**
 * pimpl
 * 实现隐藏在私有的impl实现类上，隐藏实现细节，隐藏数据成员
 */
namespace pimpl {
    // https://en.cppreference.com/w/cpp/language/pimpl
    class widget {
        class impl;
        std::unique_ptr<impl> pImpl;
    public:
        void draw() const; // public API that will be forwarded to the implementation
        void draw();
        bool shown() const { return true; } // public API that implementation has to call

        widget(); // even the default ctor needs to be defined in the implementation file
        // Note: calling draw() on default constructed object is UB
        explicit widget(int);
        ~widget(); // defined in the implementation file, where impl is a complete type
        widget(widget&&); // defined in the implementation file
        // Note: calling draw() on moved-from object is UB
        widget(const widget&) = delete;
        widget& operator=(widget&&); // defined in the implementation file
        widget& operator=(const widget&) = delete;
    };

    // ---------------------------
    // implementation (widget.cpp)
    // #include "widget.hpp"

    class widget::impl {
        int n; // private data
    public:
        void draw(const widget& w) const {
            if (w.shown()) // this call to public member function requires the back-reference 
                std::cout << "drawing a const widget " << n << '\n';
        }

        void draw(const widget& w) {
            if (w.shown())
                std::cout << "drawing a non-const widget " << n << '\n';
        }

        impl(int n) : n(n) {}
    };

    void widget::draw() const { pImpl->draw(*this); }
    void widget::draw() { pImpl->draw(*this); }
    widget::widget() = default;
    widget::widget(int n) : pImpl{ std::make_unique<impl>(n) } {}
    widget::widget(widget&&) = default;
    widget::~widget() = default;
    widget& widget::operator=(widget&&) = default;
}; // end namespace pimpl

/**
 * pimpl interface
 * 在 pimpl 的基础上，增加多态，原基类可以使用栈对象
 */
namespace pimpl_interface {

    class event final {
    public:
        class interface;
    private:
        std::unique_ptr<interface> _interface;
    public:
        explicit event(interface* ptr)
            : _interface(ptr) {
        }

        ~event() = default;

        class interface {
        public:
            virtual void on_walk() = 0;
            virtual void on_run() = 0;
        };
    };

    class role_event : event::interface {
    public:
        void on_walk() override {
            // todo:
        }
        void on_run() override {
            // todo:
        }
    };

    class monster_event : event::interface {
    public:
        void on_walk() override {
            // todo:
        }
        void on_run() override {
            // todo:
        }
    };
}; // end namespace pimpl_interface

/**
 * mixin
 * 基于不定参模版的继承写法
 * https://zhuanlan.zhihu.com/p/460825741
 */
namespace mixin {
    template <typename... Mixins>
    class Point : public Mixins... {
    public:
        double x, y;
        Point() : Mixins()..., x(0.0), y(0.0) {}
        Point(double x, double y) : Mixins()..., x(x), y(y) {}
    };

    class Label {
    public:
        std::string label;
        Label() : label("") {}
    };

    class Color {
    public:
        unsigned char red = 0, green = 0, blue = 0;
    };

    using MyPoint = Point<Label, Color>;
}; // end namespace mixin
