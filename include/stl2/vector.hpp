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
#ifndef STL2_VECTOR_HPP
#define STL2_VECTOR_HPP

#include <stl2/algorithm.hpp>
#include <stl2/iterator.hpp>
#include <stl2/type_traits.hpp>
#include <stl2/detail/ebo_box.hpp>
#include <stl2/detail/fwd.hpp>
#include <stl2/detail/concepts/allocator.hpp>

STL2_OPEN_NAMESPACE {
  struct reserve_t {};

  template <class T, ProtoAllocator<T> PA = std::allocator<T>>
  class vector : detail::ebo_box<rebind_allocator_t<PA, T>> {
    using base_t = detail::ebo_box<rebind_allocator_t<PA, T>>;
    using traits = std::allocator_traits<rebind_allocator_t<PA, T>>;
  public:
    using value_type = T;
    using allocator_type = rebind_allocator_t<PA, T>;
    using pointer = typename traits::pointer;
    using const_pointer = typename traits::const_pointer;
    using size_type = difference_type_t<pointer>;
    using iterator = pointer;
    using const_iterator = const_pointer;

    ~vector()
      requires Allocator<allocator_type, T>() &&
        AllocatorDestructible<allocator_type, T>()
    {
      clear_(begin_, end_);
      traits::deallocate(alloc(), begin_, capacity());
    }

    vector()
      noexcept(is_nothrow_default_constructible<allocator_type>::value)
      requires DefaultConstructible<allocator_type>() = default;

    vector(allocator_type a) noexcept
    : base_t{__stl2::move(a)}
    {}

    // Extension
    vector(reserve_t, size_type n, allocator_type a)
      requires Allocator<allocator_type, T>()
    : base_t{__stl2::move(a)},
      begin_{traits::allocate(alloc(), (STL2_EXPECT(n >= 0), n))},
      end_{begin_}, alloc_{begin_ + n}
    {}

    // Extension
    vector(reserve_t, size_type n)
      requires Allocator<allocator_type, T>() &&
        DefaultConstructible<allocator_type>()
    : vector{reserve_t{}, n, allocator_type{}} {}

    vector(size_type n, allocator_type a)
      requires AllocatorDefaultConstructible<allocator_type, T>()
    : vector{reserve_t{}, n, __stl2::move(a)}
    {
      while (n-- > 0) {
        emplace_back_unchecked();
      }
    }

    vector(size_type n)
      requires DefaultConstructible<allocator_type>() &&
        Allocator<allocator_type, T>() &&
        AllocatorDefaultConstructible<allocator_type, T>()
    : vector{reserve_t{}, n, allocator_type{}}
    {}

    vector(size_type n, const T& t, allocator_type a)
      requires Allocator<allocator_type, T>() &&
        AllocatorCopyConstructible<allocator_type, T>()
    : vector{reserve_t{}, n, __stl2::move(a)}
    {
      while (n-- > 0) {
        emplace_back_unchecked(t);
      }
    }

    vector(size_type n, const T& t)
      requires DefaultConstructible<allocator_type>() &&
        Allocator<allocator_type, T>() &&
        AllocatorCopyConstructible<allocator_type, T>()
    : vector{reserve_t{}, n, allocator_type{}}
    {}

    // FIXME: NYI
    vector(vector&&) = delete;
    vector(const vector&) = delete;
    vector& operator=(vector&&) & = delete;
    vector& operator=(const vector&) & = delete;

    // FIXME: everything about swap is wrong.
    void swap(vector& that)
      noexcept(is_nothrow_swappable<allocator_type&, allocator_type&>::value)
    {
      ranges::swap(alloc(), that.alloc());
      ranges::swap(begin_, that.begin_);
      ranges::swap(end_, that.end_);
      ranges::swap(alloc_, that.alloc_);
    }

    allocator_type get_allocator() const noexcept {
      return alloc();
    }

    iterator begin() noexcept { return begin_; }
    iterator end() noexcept { return end_; }

    const_iterator begin() const noexcept { return begin_; }
    const_iterator end() const noexcept { return end_; }

    auto cbegin() const noexcept { return begin(); }
    auto cend() const noexcept { return end(); }

    auto rbegin() noexcept { return reverse_iterator<iterator>{end_}; }
    auto rend() noexcept { return reverse_iterator<iterator>{begin_}; }

    auto rbegin() const noexcept { return reverse_iterator<const_iterator>{end_}; }
    auto rend() const noexcept { return reverse_iterator<const_iterator>{begin_}; }

    auto crbegin() const noexcept { return rbegin(); }
    auto crend() const noexcept { return rend(); }

    T& front() noexcept { STL2_EXPECT(!empty()); return *begin_; }
    const T& front() const noexcept { STL2_EXPECT(!empty()); return *begin_; }
    T& back() noexcept { STL2_EXPECT(!empty()); return *__stl2::prev(end_); }
    const T& back() const noexcept { STL2_EXPECT(!empty()); return *__stl2::prev(end_); }

    size_type size() const noexcept {
      return end_ - begin_;
    }
    bool empty() const noexcept {
      return end_ == begin_;
    }

    size_type capacity() const noexcept {
      return alloc_ - begin_;
    }

    void clear() noexcept
    requires
      AllocatorDestructible<allocator_type, T>()
    {
      clear_(begin_, end_);
      end_ = begin_;
    }

    // Requires n <= capacity() or *this is reallocatable
    void reserve(size_type n)
    requires
      Allocator<allocator_type, T>() &&
      AllocatorMoveConstructible<allocator_type, T>()
    {
      if (n > capacity()) {
        change_capacity(n);
      }
    }

    // Requires size() == capacity() or *this is reallocatable
    void shrink_to_fit()
    requires
      Allocator<allocator_type, T>() &&
      AllocatorMoveConstructible<allocator_type, T>()
    {
      if (end_ < alloc_) {
        change_capacity(size());
      }
    }

    // Requires n <= capacity() or *this is reallocatable
    void resize(size_type n)
    requires
      Allocator<allocator_type, T>() &&
      AllocatorDefaultConstructible<allocator_type, T>() &&
      AllocatorMoveConstructible<allocator_type, T>()
    {
      reserve(n);
      auto new_end = begin_ + n;
      if (new_end < end_) {
        clear_(new_end, end_);
        end_ = new_end;
      } else {
        for (; end_ != new_end; ++end_) {
          traits::construct(alloc(), end_);
        }
      }
    }

    // Extension
    // Requires size() < capacity()
    template <class...Args>
    requires
      AllocatorConstructible<allocator_type, T, Args...>()
    void emplace_back_unchecked(Args&&...args) {
      STL2_EXPECT(end_ != nullptr);
      STL2_EXPECT(end_ < alloc_);
      traits::construct(alloc(), end_, __stl2::forward<Args>(args)...);
      ++end_;
    }

    // Extension?
    class unchecked_back_inserter {
      detail::raw_ptr<vector> vec_;
    public:
      using difference_type = std::ptrdiff_t;

      unchecked_back_inserter() = default;
      constexpr unchecked_back_inserter(vector& vec) noexcept :
        vec_{&vec} {}

      constexpr unchecked_back_inserter& operator*() { return *this; }
      constexpr unchecked_back_inserter& operator++() & { return *this; }
      constexpr unchecked_back_inserter& operator++(int) & { return *this; }

      // requires vec_->size() < vec_->capacity()
      unchecked_back_inserter& operator=(const T& t) &
      requires
        AllocatorCopyConstructible<allocator_type, T>()
      {
        vec_->emplace_back_unchecked(t);
        return *this;
      }

      // requires vec_->size() < vec_->capacity()
      unchecked_back_inserter& operator=(T&& t) &
      requires
        AllocatorMoveConstructible<allocator_type, T>()
      {
        vec_->emplace_back_unchecked(__stl2::move(t));
        return *this;
      }
    };

    // Requires size() < capacity() or *this is reallocatable
    template <class...Args>
    requires
      Allocator<allocator_type, T>() &&
      AllocatorConstructible<allocator_type, T, Args...>() &&
      AllocatorMoveConstructible<allocator_type, T>()
    void emplace_back(Args&&...args) {
      if (end_ < alloc_) {
        emplace_back_unchecked(__stl2::forward<Args>(args)...);
      } else {
        emplace_back_slow_path(__stl2::forward<Args>(args)...);
      }
    }

    // Requires size() < capacity() or *this is reallocatable
    void push_back(const T& t)
    requires
      Allocator<allocator_type, T>() &&
      AllocatorCopyConstructible<allocator_type, T>()
    {
      emplace_back(t);
    }

    // Requires size() < capacity() or *this is reallocatable
    void push_back(T&& t)
    requires
      Allocator<allocator_type, T>() &&
      AllocatorMoveConstructible<allocator_type, T>()
    {
      emplace_back(__stl2::move(t));
    }

    void pop_back() noexcept
    requires
      AllocatorDestructible<allocator_type, T>()
    {
      STL2_EXPECT(end_ > begin_);
      traits::destroy(alloc(), std::addressof(*--end_));
    }

  private:
    pointer begin_ = nullptr;
    pointer end_ = nullptr;
    pointer alloc_ = nullptr;

    // FIXME: alloc and alloc_ are too similar.
    allocator_type& alloc() { return base_t::get(); }
    const allocator_type& alloc() const { return base_t::get(); }

    void clear_(pointer first, pointer last) noexcept
    requires
      AllocatorDestructible<allocator_type, T>()
    {
      for (; first != last; ++first) {
        traits::destroy(alloc(), std::addressof(*first));
      }
    }

    struct tmp_buf {
      using value_type = vector::value_type;

      allocator_type& a_;
      pointer begin_;
      pointer end_;
      pointer alloc_;

      ~tmp_buf()
      requires
        Allocator<allocator_type, T>() &&
        AllocatorDestructible<allocator_type, T>()
      {
        if (begin_) {
          for (auto p = begin_; p != end_; ++p) {
            traits::destroy(a_, std::addressof(*p));
          }
          traits::deallocate(a_, begin_, capacity());
        }
      }

      tmp_buf() = default;
      tmp_buf(allocator_type& a, size_type n)
      requires
        Allocator<allocator_type, T>()
      : a_{a}, begin_{traits::allocate(a, n)},
        end_{begin_}, alloc_{begin_ + n} {}

      tmp_buf(tmp_buf&& that) noexcept
      : a_{that.a_},
        begin_{__stl2::exchange(__stl2::move(that.begin_), nullptr)},
        end_{__stl2::move(that.end_)},
        alloc_{__stl2::move(that.alloc_)} {}

      tmp_buf& operator=(tmp_buf&&) & = delete;

      size_type size() const { return end_ - begin_; }
      size_type capacity() const { return alloc_ - begin_; }

      void push_back(T&& t)
      requires
        AllocatorMoveConstructible<allocator_type, T>()
      {
        STL2_EXPECT(end_ < alloc_);
        traits::construct(a_, std::addressof(*end_), __stl2::move(t));
        ++end_;
      }
    };

    struct destruct_only_deleter {
      using pointer = vector::pointer;
      allocator_type& a_;

      destruct_only_deleter(allocator_type& a) noexcept :
        a_{a} {}

      void operator() (pointer p) noexcept
      requires
        AllocatorDestructible<allocator_type, T>()
      {
        traits::destroy(a_, std::addressof(*p));
      }
    };

    void swap(tmp_buf& buf) noexcept {
      STL2_EXPECT(&buf.a_ == &alloc());
      ranges::swap(begin_, buf.begin_);
      ranges::swap(end_, buf.end_);
      ranges::swap(alloc_, buf.alloc_);
    }

    // Requires *this is reallocatable, size() == capacity()
    template <class...Args>
      requires
        Allocator<allocator_type, T>() &&
        AllocatorMoveConstructible<allocator_type, T>() &&
        AllocatorConstructible<allocator_type, T, Args...>()
    void emplace_back_slow_path(Args&&...args);

    // Requires *this is reallocatable, n >= size()
    void change_capacity(size_type n)
    requires
      Allocator<allocator_type, T>() &&
      AllocatorMoveConstructible<allocator_type, T>();

    size_type grow() const {
      auto new_capacity = __stl2::max(size_type{1}, (3 * capacity() + 1) / 2);
      STL2_EXPECT(new_capacity > capacity());
      return new_capacity;
    }
  };

  template <class T, class PA>
  template <class...Args>
  void vector<T, PA>::emplace_back_slow_path(Args&&...args)
  requires
    Allocator<allocator_type, T>() &&
    AllocatorMoveConstructible<allocator_type, T>() &&
    AllocatorConstructible<allocator_type, T, Args...>()
  {
    tmp_buf buf{alloc(), grow()};
    auto new_element_ptr = buf.begin_ + size();
    traits::construct(alloc(), std::addressof(*new_element_ptr),
                      __stl2::forward<Args>(args)...);
    auto new_element_handle =
      std::unique_ptr<T, destruct_only_deleter>{new_element_ptr, alloc()};
    __stl2::move(*this, __stl2::back_inserter(buf));
    STL2_EXPECT(buf.end_ == new_element_ptr);
    ++buf.end_;
    new_element_handle.release();
    swap(buf);
  }

  template <class T, class PA>
  void vector<T, PA>::change_capacity(size_type n)
  requires
    Allocator<allocator_type, T>() &&
    AllocatorMoveConstructible<allocator_type, T>()
  {
    STL2_EXPECT(n >= size());
    tmp_buf buf{alloc(), n};
    __stl2::move(*this, __stl2::back_inserter(buf));
    swap(buf);
  }
} STL2_CLOSE_NAMESPACE

#endif
