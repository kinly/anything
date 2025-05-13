#pragma once

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <memory> // For std::unique_ptr and std::shared_ptr
#include <queue>
#include <set>
#include <stack>
#include <string> // Included for completeness, though not strictly necessary for is_container
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <type_traits> // For std::void_t, std::false_type, std::true_type

// This header provides a utility, `common::destroy_container`,
// which is designed to recursively clear and deallocate elements
// within various C++ STL containers. It's particularly useful
// for containers that store raw pointers to dynamically allocated
// objects or nested containers of such pointers.
//
// Key features:
// - `is_container`: A type trait to detect if a type is an STL-like container.
// - `auto_delete_point`: A struct template to safely delete raw pointers
//   and reset smart pointers (std::unique_ptr, std::shared_ptr).
// - `common::container_destroyer`: A struct template with specializations
//   for various STL containers. It iterates through container elements,
//   recursively calling `destroy` on each element (if it's a pointer or
//   another container) and then clears/empties the container itself.
// - `common::destroy_container`: A helper function to invoke the destruction.
//
// Example usage (as provided in the original file):
//
//   std::queue<int *> queue_test;
//   queue_test.emplace(new int(1));
//   common::destroy_container(queue_test); // Deletes the int*
//
//   std::unordered_map<int *, std::vector<int *> *> map_test;
//   map_test.emplace(new int(1), new std::vector<int *>{new int(1), new int(2)});
//   common::destroy_container(map_test); // Deletes keys and values (including elements in the vector)
//

// Type trait to check if T is a container
// It checks for the presence of `iterator` and `value_type` nested types.
template <typename T, typename = void>
struct is_container : std::false_type {};

template <typename T>
struct is_container<T, std::void_t<typename T::iterator, typename T::value_type>> : std::true_type {};

// Forward declaration of container_destroyer in the common namespace
namespace common {
template <class T>
struct container_destroyer;
} // namespace common


// auto_delete_point: Safely deletes pointers or resets smart pointers.
// Base template: does nothing for non-pointer types.
template <class T>
struct auto_delete_point {
  static void safe_delete(const T& /*unused*/) noexcept {} // Non-pointer types are not deleted by this utility
};

// Specialization for raw pointers (T*)
template <class T>
struct auto_delete_point<T*> {
  static void safe_delete(T* P) noexcept {
    // If T itself is a container that might hold pointers,
    // recursively destroy its contents before deleting P.
    // This handles cases like T** where T is a container.
    // However, the primary destruction logic is in container_destroyer.
    // This specific check might be redundant if T is not a pointer to container.
    // The original code had a similar check in container_destroyer<T*>,
    // it's more robust to let container_destroyer handle nested destruction.
    delete P;
  }
};

// Specialization for const raw pointers (const T*)
template <class T>
struct auto_delete_point<const T*> {
  static void safe_delete(const T* P) noexcept {
    // Cast away constness to delete. This is generally safe if P was
    // originally non-const and points to dynamically allocated memory.
    delete const_cast<T*>(P);
  }
};

// Specialization for std::unique_ptr
template <class T>
struct auto_delete_point<std::unique_ptr<T>> {
  static void safe_delete(std::unique_ptr<T>& P) noexcept {
    P.reset(); // Releases ownership and deletes the managed object
  }
};

// Specialization for std::shared_ptr
template <class T>
struct auto_delete_point<std::shared_ptr<T>> {
  static void safe_delete(std::shared_ptr<T>& P) noexcept {
    P.reset(); // Decrements reference count; deletes if count reaches zero
  }
};


namespace common {

// container_destroyer: Recursively destroys elements within containers.
// Base template: does nothing for types that are not containers or pointers.
template <class T>
struct container_destroyer {
  static void destroy(const T& /*unused*/) noexcept {} // Default for non-container, non-pointer types
};

// Specialization for raw pointers (T*)
// This handles containers of pointers, e.g., std::vector<MyClass*>.
template <class T>
struct container_destroyer<T*> {
  static void destroy(T* Point) noexcept {
    if (Point == nullptr) return; // Avoid dereferencing null pointers

    // If the pointed-to type T is itself a container,
    // recursively destroy its contents.
    // Example: std::vector<MyClass*>* ptr_to_vector;
    if constexpr (is_container<T>::value) {
      container_destroyer<T>::destroy(*Point);
    }
    // After potentially destroying contents of a container pointed to,
    // delete the pointer itself.
    auto_delete_point<T*>::safe_delete(Point);
  }
};

// Specialization for const raw pointers (const T*)
template <class T>
struct container_destroyer<const T*> {
  static void destroy(const T* Point) noexcept {
    if (Point == nullptr) return;

    if constexpr (is_container<T>::value) {
      // Need to cast away const to destroy contents if T is a container
      // This assumes the container's destroy method can handle a non-const reference
      container_destroyer<T>::destroy(*const_cast<T*>(Point));
    }
    auto_delete_point<const T*>::safe_delete(Point);
  }
};

// Specialization for std::unique_ptr<T>
// This handles containers of unique_ptrs, e.g., std::vector<std::unique_ptr<MyClass>>.
template <class T>
struct container_destroyer<std::unique_ptr<T>> {
  static void destroy(std::unique_ptr<T>& Ptr) noexcept {
    if (!Ptr) return;

    // If the managed object T is itself a container, destroy its contents.
    if constexpr (is_container<T>::value) {
      container_destroyer<T>::destroy(*Ptr);
    }
    // Then reset the unique_ptr, which deletes the managed object.
    // auto_delete_point handles this, but can also be Ptr.reset().
    auto_delete_point<std::unique_ptr<T>>::safe_delete(Ptr);
  }
};

// Specialization for std::shared_ptr<T>
// This handles containers of shared_ptrs, e.g., std::vector<std::shared_ptr<MyClass>>.
template <class T>
struct container_destroyer<std::shared_ptr<T>> {
  static void destroy(std::shared_ptr<T>& Ptr) noexcept {
    if (!Ptr) return;

    // If the managed object T is a container, destroy its contents.
    // This is important if the shared_ptr is the sole owner or
    // if we want to ensure deep cleanup before the last reference is dropped.
    if constexpr (is_container<T>::value) {
      // Only destroy contents if this is the last owner,
      // or if the intent is to clear contents regardless of other owners.
      // The current approach will attempt to destroy contents.
      // If T contains raw pointers, they will be deleted.
      // If T contains other smart pointers, their destructors will handle them.
      // A check like `if (Ptr.unique())` could be added if destruction
      // should only happen for the last owner. However, the current design
      // seems to aim for clearing contents pointed to by elements.
      container_destroyer<T>::destroy(*Ptr);
    }
    // Then reset the shared_ptr.
    auto_delete_point<std::shared_ptr<T>>::safe_delete(Ptr);
  }
};


// Specializations for specific STL containers

// std::vector
template <class V, class A>
struct container_destroyer<std::vector<V, A>> {
  static void destroy(std::vector<V, A>& C) {
    // Recursively destroy each element in the vector.
    // If V is a pointer type (e.g., int*), container_destroyer<int*>::destroy will be called.
    // If V is a value type (e.g., int), container_destroyer<int>::destroy (base template) will be called (no-op).
    // If V is another container (e.g., std::vector<int*>), container_destroyer<std::vector<int*>>::destroy will be called.
    for (auto& item : C) {
      container_destroyer<V>::destroy(item);
    }
    C.clear(); // Removes all elements from the vector.
               // For non-pointer types, destructors are called.
               // For pointer types, this just removes pointers, memory must be managed.
               // The loop above handles deallocation for pointer types.
  }
};

// std::list
template <class V, class A>
struct container_destroyer<std::list<V, A>> {
  static void destroy(std::list<V, A>& C) {
    for (auto& item : C) {
      container_destroyer<V>::destroy(item);
    }
    C.clear();
  }
};

// std::deque
template <class V, class A>
struct container_destroyer<std::deque<V, A>> {
  static void destroy(std::deque<V, A>& C) {
    for (auto& item : C) {
      container_destroyer<V>::destroy(item);
    }
    C.clear();
  }
};

// std::array
// Note: std::array elements are part of the array object itself.
// `clear()` is not a member. We just destroy elements.
template <class T, std::size_t N>
struct container_destroyer<std::array<T, N>> {
  static void destroy(std::array<T, N>& C) {
    for (auto& item : C) {
      container_destroyer<T>::destroy(item);
    }
    // No C.clear() for std::array. Elements are destroyed in place.
    // If T is a class type, its destructor is called.
    // If T is a pointer, the loop above handles deletion.
  }
};

// std::forward_list
template <class V, class A>
struct container_destroyer<std::forward_list<V, A>> {
  static void destroy(std::forward_list<V, A>& C) {
    for (auto& item : C) {
      container_destroyer<V>::destroy(item);
    }
    C.clear(); // Empties the forward_list
  }
};

// std::queue
// Queue is an adaptor, typically over deque or list.
// It does not have iterators. We must use front() and pop().
template <class T, class Container>
struct container_destroyer<std::queue<T, Container>> {
  static void destroy(std::queue<T, Container>& C) {
    while (!C.empty()) {
      // For std::queue, C.front() returns a reference.
      // If T is a pointer type (e.g. int*), we need to destroy the pointed-to object.
      // If T is a value type, its destructor will be called upon pop if it's complex,
      // but for pointer types, explicit destruction is needed.
      // The container_destroyer<T>::destroy call handles this.
      // Note: If T is `const SomeType*`, `C.front()` might return `const SomeType* const&`.
      // We need to ensure `container_destroyer` can handle `const` if necessary,
      // or that the queue stores mutable types if deep modification/deletion is intended.
      // The current setup with `container_destroyer<const T*>` should handle this.
      // For `std::queue<T>`, `front()` returns `T&`.
      // For `std::queue<T*>`, `front()` returns `T*&`.
      container_destroyer<T>::destroy(C.front()); // Process the element
      C.pop(); // Removes the element
    }
  }
};

// std::stack
// Stack is an adaptor, typically over deque or list.
// It does not have iterators. We must use top() and pop().
template <class T, class Container>
struct container_destroyer<std::stack<T, Container>> {
  static void destroy(std::stack<T, Container>& C) {
    while (!C.empty()) {
      // Similar to queue, C.top() returns a reference.
      container_destroyer<T>::destroy(C.top());
      C.pop();
    }
  }
};

// std::priority_queue
// Priority queue is an adaptor.
// It does not have iterators. We must use top() and pop().
// Elements are typically const-qualified when accessed via top().
template <class T, class Container, class Compare> // Added Compare template parameter
struct container_destroyer<std::priority_queue<T, Container, Compare>> {
  static void destroy(std::priority_queue<T, Container, Compare>& C) {
    while (!C.empty()) {
      // C.top() returns `const T&`.
      // If T is `int*`, `C.top()` returns `int* const&`.
      // `container_destroyer<T>::destroy` will be called.
      // If T is `int*`, then `container_destroyer<int*>::destroy(int* const&)` is called.
      // This needs to correctly resolve to `container_destroyer<int*>::destroy(int*)`.
      // The `const_cast` might be needed if `destroy` expects a non-const argument
      // for modification (like resetting a smart pointer).
      // However, our `container_destroyer<V>::destroy(item)` pattern usually passes `item` by reference,
      // and `auto_delete_point` specializations for smart pointers take them by reference.
      // For raw pointers, `T*` or `const T*` specializations are used.
      // `container_destroyer<T>::destroy(C.top())` should work if T itself is not const-qualified in the priority_queue's definition.
      // E.g., `priority_queue<int*>`: `top()` is `int* const&`. `container_destroyer<int*>::destroy` is fine.
      // E.g., `priority_queue<MyClass>`: `top()` is `const MyClass&`. `container_destroyer<MyClass>::destroy` is fine.
      // The key is that `container_destroyer` for pointer types handles the deletion.
      // The original code had `container_destroyer<T>::destroy(C.top());`
      // For `std::priority_queue<int*, std::vector<int*>, std::greater<int*>> pq;`
      // `pq.top()` is `int* const&`. `T` is `int*`. `container_destroyer<int*>::destroy` is called.
      // This seems correct.
      container_destroyer<T>::destroy(const_cast<T&>(C.top())); // const_cast needed if T is a pointer type that destroy modifies (e.g. smart pointer reset) or if T is a container whose elements are modified.
                                                              // For raw pointers, the pointer value itself is passed, and deletion happens on that value.
                                                              // For smart pointers (unique_ptr, shared_ptr), they are typically stored by value in containers.
                                                              // If `T` is `std::unique_ptr<Foo>`, `C.top()` is `const std::unique_ptr<Foo>&`.
                                                              // `container_destroyer<std::unique_ptr<Foo>>::destroy` needs a `std::unique_ptr<Foo>&`.
                                                              // So const_cast is necessary here.
      C.pop();
    }
  }
};

// std::set
// For sets, keys are const. If V is a pointer, the pointer itself is const, not the pointed-to object.
// e.g., std::set<int*>. `item` is `int* const&`. `container_destroyer<int*>::destroy` is called.
template <class V, class P, class A>
struct container_destroyer<std::set<V, P, A>> {
  static void destroy(std::set<V, P, A>& C) {
    for (const auto& item : C) { // item is of type const V& or V if V is a pointer
                                 // If V = int*, item is int* (a copy of the pointer).
                                 // If V = MyClass, item is const MyClass&.
                                 // The loop iterates over copies for pointers, references for objects.
                                 // To be safe and consistent, iterate by reference.
      container_destroyer<V>::destroy(const_cast<V&>(item)); // const_cast if V is a pointer type that destroy modifies (smart pointer reset)
                                                             // or if V is a container that destroy modifies.
                                                             // For `std::set<int*>`, V is `int*`. `item` is `int*`.
                                                             // `container_destroyer<int*>::destroy(item)` is fine.
                                                             // For `std::set<std::unique_ptr<MyClass>>`, this is not allowed as unique_ptr is not copyable.
                                                             // Sets require CopyConstructible and Comparable keys.
                                                             // If `std::set<MyClass* my_compare>`, then V is `MyClass*`.
                                                             // `for (MyClass* const& item : C)`
                                                             // `container_destroyer<MyClass*>::destroy(item)` is fine.
                                                             // The original code `for (auto& item : C)` for set is problematic because set iterators are const.
                                                             // It should be `for (const auto& item : C)`.
                                                             // Then, if `V` is `std::unique_ptr`, `destroy` needs a non-const ref.
                                                             // However, `std::set<std::unique_ptr<T>>` is not directly possible due to copyability requirements for insertion/rebalancing.
                                                             // Assuming V is a raw pointer or a type that can be handled by const_cast if necessary.
    }
    // The loop below is more standard for sets where elements are const.
    // However, to call destroy which might modify (e.g. reset a smart pointer stored by value,
    // or delete a raw pointer), we need to handle the constness.
    // If V is a raw pointer `T*`, `item` in `for (auto item : C)` is `T*`.
    // `container_destroyer<T*>::destroy(item)` works.
    // The original `for (auto& item : C)` would fail to compile as set iterators are const iterators.
    // Corrected loop:
    for (auto it = C.begin(); it != C.end(); ++it) {
        // *it is of type const V.
        // If V is T*, then *it is T* (const).
        // We need to pass a V to container_destroyer<V>::destroy.
        // If V is T*, we pass T*.
        // If V is std::unique_ptr<T> (not possible with set directly), it would be const std::unique_ptr<T>&.
        // The main concern is if V itself is a smart pointer type that needs non-const access for reset.
        // For raw pointers, passing the pointer value is fine.
        container_destroyer<V>::destroy(*it); // This relies on container_destroyer<V> being able to handle const V if V is not a pointer.
                                              // Or, if V is a pointer T*, it passes T*.
                                              // If V is `Foo*`, `*it` is `Foo*`. `container_destroyer<Foo*>::destroy(*it)` is called.
    }
    C.clear();
  }
};


// std::unordered_set
template <class V, class H, class P, class A> // Corrected H for Hash
struct container_destroyer<std::unordered_set<V, H, P, A>> {
  static void destroy(std::unordered_set<V, H, P, A>& C) {
    // Similar to std::set, iterators are const.
    // `for (auto& item : C)` is problematic.
    // Should be `for (const auto& item : C)` or iterate and pass `*it`.
    for (auto it = C.begin(); it != C.end(); ++it) {
        container_destroyer<V>::destroy(*it);
    }
    C.clear();
  }
};

// std::map
// For maps, keys are const, values are not.
// `std::map<const K, V>` effectively.
template <class K, class V, class P, class A>
struct container_destroyer<std::map<K, V, P, A>> {
  static void destroy(std::map<K, V, P, A>& C) {
    for (auto& pair : C) { // pair is std::pair<const K, V>&
      // Destroy key (if it's a pointer type that needs destruction)
      // K is the key type. If K is `int*`, then `pair.first` is `int* const &`.
      // `container_destroyer<K>::destroy` will be `container_destroyer<int*>::destroy`.
      container_destroyer<K>::destroy(pair.first);

      // Destroy value (if it's a pointer type or another container)
      // V is the value type. `pair.second` is `V&`.
      container_destroyer<V>::destroy(pair.second);
    }
    C.clear();
  }
};

// std::unordered_map
template <class K, class V, class H, class P, class A>
struct container_destroyer<std::unordered_map<K, V, H, P, A>> {
  static void destroy(std::unordered_map<K, V, H, P, A>& C) {
    for (auto& pair : C) { // pair is std::pair<const K, V>&
      container_destroyer<K>::destroy(pair.first);
      container_destroyer<V>::destroy(pair.second);
    }
    C.clear();
  }
};


// Global helper function to destroy a container
// D is the destroyer policy, defaults to container_destroyer
template <class T, template <class> class D = container_destroyer>
void destroy_container(T& v) {
  // This check ensures that we only attempt to destroy actual containers.
  // If T is not a container (e.g., int, MyClass), D<T>::destroy will
  // (by default) call the base container_destroyer<T>::destroy which is a no-op.
  // This is good, as it prevents errors if someone calls destroy_container on a non-container.
  // However, if T is a pointer *to* a container (e.g. std::vector<int*>*),
  // is_container<T> will be false. The destruction for pointers is handled by
  // container_destroyer<T*>, which then checks if *T is a container.
  if constexpr (is_container<T>::value || std::is_pointer<T>::value) {
      D<T>::destroy(v);
  }
  // If T is not a container and not a pointer, D<T>::destroy(v) will be a no-op by default.
  // The `if constexpr` could be removed if relying on the default no-op behavior is acceptable.
  // The original code did not have this `if constexpr` here, relying on the SFINAE/specializations.
  // Keeping it explicit might be clearer for the top-level function.
  // Let's stick to the original structure where D<T>::destroy is always called.
  D<T>::destroy(v);
}

} // namespace common


/*
 * Example Usage:
 *
 * #include <iostream> // For example main
 *
 * struct MyResource {
 * int id;
 * MyResource(int i) : id(i) { std::cout << "MyResource " << id << " created\n"; }
 * ~MyResource() { std::cout << "MyResource " << id << " destroyed\n"; }
 * };
 *
 * int main() {
 * {
 * std::cout << "--- Test 1: std::vector<int*> ---" << std::endl;
 * std::vector<int*> vec_ptr;
 * vec_ptr.push_back(new int(10));
 * vec_ptr.push_back(new int(20));
 * common::destroy_container(vec_ptr);
 * std::cout << "vec_ptr.empty(): " << std::boolalpha << vec_ptr.empty() << std::endl;
 * }
 *
 * {
 * std::cout << "\n--- Test 2: std::queue<MyResource*> ---" << std::endl;
 * std::queue<MyResource*> queue_test;
 * queue_test.emplace(new MyResource(1));
 * queue_test.emplace(new MyResource(2));
 * common::destroy_container(queue_test);
 * std::cout << "queue_test.empty(): " << std::boolalpha << queue_test.empty() << std::endl;
 * }
 *
 * {
 * std::cout << "\n--- Test 3: std::unordered_map<int*, std::vector<MyResource*>*> ---" << std::endl;
 * std::unordered_map<int*, std::vector<MyResource*>*> map_test;
 * auto* key1 = new int(1);
 * auto* val_vec1 = new std::vector<MyResource*>{new MyResource(101), new MyResource(102)};
 * map_test.emplace(key1, val_vec1);
 *
 * auto* key2 = new int(2);
 * auto* val_vec2 = new std::vector<MyResource*>{new MyResource(201)};
 * map_test.emplace(key2, val_vec2);
 *
 * common::destroy_container(map_test);
 * std::cout << "map_test.empty(): " << std::boolalpha << map_test.empty() << std::endl;
 * }
 *
 * {
 * std::cout << "\n--- Test 4: std::vector<std::unique_ptr<MyResource>> ---" << std::endl;
 * std::vector<std::unique_ptr<MyResource>> vec_unique;
 * vec_unique.push_back(std::make_unique<MyResource>(301));
 * vec_unique.push_back(std::make_unique<MyResource>(302));
 * common::destroy_container(vec_unique); // Elements are unique_ptrs, they will be reset.
 * std::cout << "vec_unique.empty(): " << std::boolalpha << vec_unique.empty() << std::endl;
 * }
 *
 * {
 * std::cout << "\n--- Test 5: std::list<std::shared_ptr<MyResource>> ---" << std::endl;
 * std::list<std::shared_ptr<MyResource>> list_shared;
 * list_shared.push_back(std::make_shared<MyResource>(401));
 * auto shared_res = std::make_shared<MyResource>(402);
 * list_shared.push_back(shared_res);
 * // common::destroy_container(list_shared); // Resets shared_ptrs in the list.
 * // If 'shared_res' is the last owner of MyResource(402) after list_shared is destroyed,
 * // then MyResource(402) will be deleted when 'shared_res' goes out of scope or is reset.
 * // The destroy_container will reset the shared_ptrs within the list.
 * common::destroy_container(list_shared);
 * std::cout << "list_shared.empty(): " << std::boolalpha << list_shared.empty() << std::endl;
 * std::cout << "shared_res use_count after destroy: " << shared_res.use_count() << std::endl;
 * }
 *
 * {
 * std::cout << "\n--- Test 6: Pointer to container ---" << std::endl;
 * std::vector<int*>* ptr_to_vec = new std::vector<int*>();
 * ptr_to_vec->push_back(new int(50));
 * ptr_to_vec->push_back(new int(60));
 * // common::destroy_container(*ptr_to_vec); // This would clear the vector but not delete the vector itself
 * // delete ptr_to_vec;                   // This would delete the vector (leaking ints)
 *
 * common::destroy_container(ptr_to_vec); // This should destroy elements and then the vector itself.
 * // ptr_to_vec is now a dangling pointer.
 * std::cout << "ptr_to_vec was destroyed." << std::endl;
 * }
 *
 * return 0;
 * }
 *
 */
