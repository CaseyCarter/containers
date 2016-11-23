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
    template <PointerTo<void> VoidPointer>
    class link {
      using pointer = rebind_pointer_t<VoidPointer, link>;

      explicit operator bool() const noexcept {
        return next_ != nullptr;
      }

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

    private:
      pointer next_;
    };

    template <class T, PointerTo<void> VoidPointer>
    class node : link<VoidPointer> {
    public:
      using pointer = rebind_pointer_t<VoidPointer, node>;
      using link_t = link<VoidPointer>;
      using link_pointer = typename link_t::pointer;

      static link_pointer as_link(const pointer& p) noexcept {
        STL2_EXPECT(p);
        return __stl2::pointer_to<link_pointer>(static_cast<link_t&>(*p));
      }

      static pointer as_node(const link_pointer& p) noexcept {
        STL2_EXPECT(p);
        return __stl2::pointer_to<pointer>(static_cast<node&>(*p));
      }

      T& get() noexcept { return reinterpret_cast<T&>(storage_); }
      const T& get() const noexcept { return reinterpret_cast<const T&>(storage_); }

      pointer next() const noexcept {
        if (auto lnext = link_t::next()) {
          return as_node(lnext);
        } else {
          return nullptr;
        }
      }

      void push(pointer new_next) noexcept {
        STL2_EXPECT(new_next);
        link_t::push(as_link(new_next));
      }

    private:
      aligned_storage<sizeof(T), alignof(T)> storage_;
    };

    template <class T, PointerTo<void> VoidPointer>
    struct cursor {
      using value_type = remove_cv_t<T>;
      using node_t = node<value_type, VoidPointer>;
      using link_t = link<VoidPointer>;
      using link_pointer = typename link_t::pointer;
      using difference_type = difference_type_t<link_pointer>;

      cursor() = default;
      constexpr cursor(default_sentinel) noexcept
      : pos_{nullptr} {}
      constexpr cursor(const cursor<remove_const_t<T>, VoidPointer>& that) noexcept
      requires !std::is_const<T>::value
      : pos_{that.pos_} {}

      T& read() const noexcept {
        STL2_EXPECT(pos_);
        return node_t::as_node(pos_)->get();
      }
      void next() noexcept {
        STL2_EXPECT(pos_);
        pos_ = pos_->next();
      }
      constexpr bool equal(const cursor& that) const noexcept {
        return pos_ == that.pos_;
      }
      constexpr bool done() const noexcept {
        return pos_ == nullptr;
      }
    private:
      explicit constexpr cursor(link_pointer pos) noexcept
      : pos_{__stl2::move(pos)} {}

      link_pointer pos_;
    };

    template <class T, PointerTo<void> VoidPointer>
    class base {
    protected:
      using link_t = link<VoidPointer>;
      link_t head_{};

      template <class U>
      using cursor = __fl::cursor<U, VoidPointer>;

    public:
      using value_type = T;
      using iterator = __stl2::basic_iterator<cursor<T>>;
      using const_iterator = __stl2::basic_iterator<cursor<const T>>;

      iterator before_begin() noexcept {
        return cursor<T>{
          __stl2::pointer_to<typename link_t::pointer>(head_)};
      }
      const_iterator before_begin() const noexcept {
        return cursor<const T>{
          __stl2::pointer_to<typename link_t::pointer>(head_)};
      }
      const_iterator cbefore_begin() const noexcept {
        return before_begin();
      }

      iterator begin() noexcept {
        return cursor<T>{head_.next()};
      }
      const_iterator begin() const noexcept {
        return cursor<const T>{head_.next()};
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
        return *begin();
      }
      const T& front() const noexcept {
        STL2_EXPECT(head_);
        return *begin();
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
      head_.push(node_t::as_link(new_node));
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
      Same<node_pointer> tmp = node_t::as_node(head_.pop());
      traits::destroy(alloc, std::addressof(tmp->get()));
      traits::deallocate(alloc, tmp, 1);
    }
  };
} STL2_CLOSE_NAMESPACE

#endif
