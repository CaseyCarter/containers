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
#ifndef STL2_FORWARD_LIST_HPP
#define STL2_FORWARD_LIST_HPP

#include <stl2/algorithm.hpp>
#include <stl2/iterator.hpp>
#include <stl2/type_traits.hpp>
#include <stl2/detail/ebo_box.hpp>
#include <stl2/detail/fwd.hpp>
#include <stl2/detail/concepts/allocator.hpp>
#include <memory>

STL2_OPEN_NAMESPACE {
  namespace __fl {
    template <class T, PointerTo<void> VoidPointer>
    struct node {
      using pointer = rebind_pointer_t<VoidPointer, node>;

      T& get() & noexcept { return reinterpret_cast<T&>(storage_); }
      const T& get() const& noexcept { return reinterpret_cast<const T&>(storage_); }

      pointer next() const noexcept {
        return next_;
      }

      void push(pointer p) noexcept {
        STL2_EXPECT(p);
        p->next_ = __stl2::move(next_);
        next_ = __stl2::move(p);
      }

      pointer pop() noexcept {
        STL2_EXPECT(next_);
        return __stl2::exchange(next_, next_->next_);
      }

      pointer next_;
      aligned_storage_t<sizeof(T), alignof(T)> storage_;
    };

    template <class T, PointerTo<void> VoidPointer>
    struct cursor {
      using value_type = remove_cv_t<T>;
      using node_t = node<value_type, VoidPointer>;
      using node_pointer = rebind_pointer_t<VoidPointer, node_t>;
      using difference_type = std::ptrdiff_t;

      cursor() = default;
      constexpr cursor(default_sentinel) noexcept
      : pos_{nullptr} {}
      template <class U>
      requires
        !std::is_const<T>::value &&
        Same<U, remove_const_t<T>>()
      constexpr cursor(const cursor<U, VoidPointer>& that) noexcept
      : pos_{that.pos_} {}

      T& read() const noexcept {
        STL2_EXPECT(pos_);
        static_assert(std::is_standard_layout<node_t>::value);
        static_assert(offsetof(node_t, next_) == 0);
        return reinterpret_cast<node_t*>(pos_)->get();
      }
      void next() noexcept {
        STL2_EXPECT(pos_);
        pos_ = std::addressof((*pos_)->next_);
      }
      constexpr bool equal(const cursor& that) const noexcept {
        return pos_ == that.pos_;
      }
      constexpr bool done() const noexcept {
        return pos_ == nullptr;
      }
    private:
      explicit constexpr cursor(node_pointer* pos) noexcept
      : pos_{__stl2::move(pos)} {}

      node_pointer* pos_;
    };

    template <class T, PointerTo<void> VoidPointer>
    class base {
    protected:
      rebind_pointer_t<VoidPointer, node<T, VoidPointer>> head_ = nullptr;

    public:
      using value_type = T;
      using iterator = __stl2::basic_iterator<cursor<T, VoidPointer>>;
      using const_iterator = __stl2::basic_iterator<cursor<const T, VoidPointer>>;

      iterator before_begin() noexcept {
        return cursor<T, VoidPointer>{std::addressof(head_)};
      }
      const_iterator before_begin() const noexcept {
        return cursor<const T, VoidPointer>{std::addressof(head_)};
      }
      const_iterator cbefore_begin() const noexcept {
        return before_begin();
      }

      iterator begin() noexcept {
        return cursor<T, VoidPointer>{head_ ? std::addressof(head_->next_): nullptr};
      }
      const_iterator begin() const noexcept {
        return cursor<const T, VoidPointer>{head_ ? std::addressof(head_->next_) : nullptr};
      }
      const_iterator cbegin() const noexcept {
        return begin();
      }

      default_sentinel end() const noexcept {
        return {};
      }
      default_sentinel cend() const noexcept {
        return {};
      }

      T& front() noexcept {
        STL2_EXPECT(head_);
        return head_->get();
      }
      const T& front() const noexcept {
        STL2_EXPECT(head_);
        return head_->get();
      }
    };
  }

  template <class T, ProtoAllocator A = std::allocator<T>>
  requires
    ProtoAllocator<A, __fl::node<T, proto_allocator_pointer_t<A>>>()
  class forward_list
    : public __fl::base<T, proto_allocator_pointer_t<A>>
    , detail::ebo_box<A>
  {
    using base_t = __fl::base<T, proto_allocator_pointer_t<A>>;
    using base_t::head_;

    using node_t = __fl::node<T, proto_allocator_pointer_t<A>>;
    using node_allocator_type = rebind_allocator_t<A, node_t>;
    using node_pointer = allocator_pointer_t<node_allocator_type>;
    using traits = std::allocator_traits<node_allocator_type>;

  public:
    using allocator_type = A;

    ~forward_list()
    requires
      Allocator<node_allocator_type, node_t>() &&
      AllocatorDestructible<node_allocator_type, T>()
    {
      while (head_) {
        pop_front();
      }
    }

    forward_list()
    requires DefaultConstructible<A>() = default;

    forward_list(allocator_type a) noexcept
    : detail::ebo_box<A>(__stl2::move(a)) {}

    // FIXME: NYI
    forward_list(forward_list&&) = delete;
    forward_list(const forward_list&) = delete;
    forward_list& operator=(forward_list&&) & = delete;
    forward_list& operator=(const forward_list&) & = delete;

    // FIXME: NYI
    void swap(forward_list& that);
    friend void swap(forward_list& lhs, forward_list& rhs)
    STL2_NOEXCEPT_RETURN(
      lhs.swap(rhs)
    )

    allocator_type get_allocator() const noexcept {
      return detail::ebo_box<A>::get();
    }

    template <class...Args>
    requires
      Allocator<node_allocator_type, node_t>() &&
      AllocatorConstructible<node_allocator_type, T, Args...>()
    void emplace_front(Args&&...args) {
      auto alloc = node_allocator_type{detail::ebo_box<A>::get()};
      Same<node_pointer> new_node = traits::allocate(alloc, 1);
      try {
        traits::construct(alloc, std::addressof(new_node->get()),
                          __stl2::forward<Args>(args)...);
      } catch(...) {
        traits::deallocate(alloc, new_node, 1);
        throw;
      }
      new_node->next_ = head_;
      head_ = new_node;
    }

    void push_front(const T& t)
    requires
      Allocator<node_allocator_type, node_t>() &&
      AllocatorCopyConstructible<node_allocator_type, T>()
    {
      emplace_front(t);
    }
    void push_front(T&& t)
    requires
      Allocator<node_allocator_type, node_t>() &&
      AllocatorMoveConstructible<node_allocator_type, T>()
    {
      emplace_front(__stl2::move(t));
    }

    void pop_front() noexcept
    requires
      Allocator<node_allocator_type, node_t>() &&
      AllocatorDestructible<node_allocator_type, T>()
    {
      STL2_EXPECT(head_);
      auto alloc = node_allocator_type{detail::ebo_box<A>::get()};
      node_pointer tmp = head_;
      head_ = head_->next_;
      traits::destroy(alloc, std::addressof(tmp->get()));
      traits::deallocate(alloc, tmp, 1);
    }
  };
} STL2_CLOSE_NAMESPACE

#endif
