// cmcstl2 - A concept-enabled C++ standard library
//
//  Copyright Casey Carter 2015
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
#include <stl2/detail/concepts/allocator.hpp>
#include <cstdlib>

namespace ranges = std::experimental::ranges;

template <class T>
class mallocator {
public:
  using value_type = T;
  using propagate_on_container_swap = ranges::true_type;
  using propagate_on_container_move_assignment = ranges::true_type;

  mallocator() = default;
  constexpr mallocator(const mallocator<auto>&) noexcept {}

  constexpr std::size_t max_size() const noexcept {
    return std::size_t(-1) / sizeof(T);
  }

  void deallocate(T* ptr, std::size_t) const noexcept {
    std::free(ptr);
  }

  template <class U = T>
  requires
    ranges::Same<U, T>() &&
    (alignof(U) <= alignof(std::max_align_t))
  U* allocate(std::size_t n) const
  {
    if (n <= max_size()) {
      if (auto vptr = std::malloc(n * sizeof(U))) {
        return static_cast<U*>(vptr);
      }
    }
    throw std::bad_alloc{};
  }

  template <class U = T>
  requires
    ranges::Same<U, T>() &&
    alignof(U) > alignof(std::max_align_t) &&
    requires { { ::aligned_alloc(alignof(U), sizeof(U)) } -> void*; }
  U* allocate(std::size_t n) const
  {
    static_assert(alignof(U) & (alignof(U) - 1) == 0);
    static_assert(sizeof(U) % alignof(U) == 0);

    if (n <= max_size()) {
      if (void* vptr = ::aligned_alloc(alignof(U), n * sizeof(U))) {
        return static_cast<U*>(vptr);
      }
    }
    throw std::bad_alloc{};
  }
};

template <>
class mallocator<void> {};

constexpr bool operator==(mallocator<auto>, mallocator<auto>) noexcept {
  return true;
}
constexpr bool operator!=(mallocator<auto>, mallocator<auto>) noexcept {
  return false;
}

static_assert(ranges::models::ProtoAllocator<mallocator<void>>);
static_assert(ranges::models::ProtoAllocator<mallocator<void>, int>);
static_assert(ranges::models::Same<mallocator<int>,
                ranges::rebind_allocator_t<mallocator<void>, int>>);
static_assert(ranges::models::Allocator<mallocator<int>, int>);

struct incomplete;
static_assert(ranges::models::ProtoAllocator<mallocator<void>, incomplete>);
static_assert(ranges::models::Same<mallocator<incomplete>,
                ranges::rebind_allocator_t<mallocator<void>, incomplete>>);
struct incomplete {};
static_assert(ranges::models::Allocator<mallocator<incomplete>, incomplete>);

int main() {}
