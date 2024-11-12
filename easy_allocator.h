#pragma once
#include <cstdint>
#include <stack>
#include <vector>
#include <memory>

namespace easy {
namespace alloc {

static constexpr int32_t size_unlimited = -1;

template <class object_tt, int32_t size_vv>
class allocator {
 public:
  using object_type = std::decay_t<object_tt>;
  using object_ptr = std::unique_ptr<object_type>;

 protected:
  std::vector<object_ptr> _alloceds;

 private:
  object_ptr __allocate() {
    if (!_alloceds.empty()) {
      object_ptr result = std::move(_alloceds.back());
      _alloceds.pop_back();
      return result;
    }
    else {
      return object_ptr(
          static_cast<object_type*>(::operator new(sizeof(object_tt))));
    }
  }

 public:
  allocator() { _alloceds.reserve(size_vv); }

  virtual ~allocator() {
    _alloceds.clear();
  }

  template <typename... args_tt>
  object_type* allocate(args_tt&&... args) {
    object_ptr place = __allocate();
    try {
      new (place.get()) object_tt(std::forward<args_tt>(args)...);
    } catch (...) {
      _alloceds.push_back(std::move(place));
      throw;
    }
    return place.release();
  }

  void deallocate(object_type* obj) {
    obj->~object_tt();

    if (_alloceds.size() >= size_vv) {
      ::operator delete(obj);
    }
    else {
      _alloceds.push_back(std::unique_ptr<object_type>(obj));
    }
  }

  template <typename... args_tt>
  std::shared_ptr<object_type> allocate_shared(args_tt&&... args) {
    auto ptr = allocate(std::forward<args_tt>(args)...);
    if (ptr == nullptr)
      return nullptr;
    return std::shared_ptr<object_type>(ptr, [this](object_type* ptr) {
      this->deallocate(ptr);
    });
  }
};

template <class object_tt>
class allocator<object_tt, size_unlimited> {
 public:
  using object_type = std::decay_t<object_tt>;
  using object_ptr = std::unique_ptr<object_type>;

 protected:
  std::stack<object_ptr> _alloceds;

 private:
  object_ptr __allocate() {
    if (!_alloceds.empty()) {
      object_ptr result = std::move(_alloceds.top());
      _alloceds.pop();
      return result;
    }
    else {
      return object_ptr(
          static_cast<object_type*>(::operator new(sizeof(object_tt))));
    }
  }

 public:
  allocator() {}

  virtual ~allocator() {
    while (!_alloceds.empty()) {
      _alloceds.pop();
    }
  }

  template <typename... args_tt>
  object_type* allocate(args_tt&&... args) {
    object_ptr place = __allocate();
    try {
      new (place.get()) object_tt(std::forward<args_tt>(args)...);
    } catch (...) {
      _alloceds.push(std::move(place));
      throw;
    }
    return place.release();
  }

  void deallocate(object_type* obj) {
    obj->~object_tt();
    _alloceds.push(std::unique_ptr<object_type>(obj));
  }

  template <typename... args_tt>
  std::shared_ptr<object_type> allocate_shared(args_tt&&... args) {
    auto ptr = allocate(std::forward<args_tt>(args)...);
    if (ptr == nullptr)
      return nullptr;
    return std::shared_ptr<object_type>(ptr, [this](object_type* ptr) {
      this->deallocate(ptr);
    });
  }
};
}  // namespace alloc
}  // namespace easy

/*
// 如果想增加 unique_ptr 支持，需要这么做
// 因为 unique_ptr 的 deleter 需要在模板参数中明确指定....
// 但是感觉这样的返回值类型可能对于大部分使用者都不是个方便的样子，就没有把这部分写入代码
template <typename... args_tt>
std::unique_ptr<object_type, std::function<void(object_type*)>> allocate_unique(args_tt&&... args) {
   auto ptr = allocate(std::forward<args_tt>(args)...);
   return std::unique_ptr<object_type, std::function<void(object_type*)>>(ptr, [this](object_type* ptr) {
       this->deallocate(ptr);
   });
}
*/


/*
 *
void alloc_test() {
  easy::alloc::allocator<timer_cost, -1> ulimited_alloc;
  easy::alloc::allocator<timer_cost, 5> limited_alloc;

  std::set<timer_cost*> allocs;
  for (int i = 0; i < 100; ++i) {
    auto one = ulimited_alloc.allocate();

    if (i % 5 == 0) {
      ulimited_alloc.deallocate(one);
      continue;
    }

    allocs.emplace(std::move(one));
  }

  for (auto one : allocs) {
    ulimited_alloc.deallocate(one);
  }
  allocs.clear();

  for (int i = 0; i < 100; ++i) {
    auto one = limited_alloc.allocate();

    if (i % 5 == 0) {
      limited_alloc.deallocate(one);
      continue;
    }

    allocs.emplace(one);
  }

  for (auto one : allocs) {
    limited_alloc.deallocate(one);
  }

  std::set<std::shared_ptr<timer_cost>> unique_allocs;
  for (int i = 0; i < 100; ++i) {
    auto one = ulimited_alloc.allocate_shared();

    unique_allocs.emplace(std::move(one));
  }
  unique_allocs.clear();
}
 */
