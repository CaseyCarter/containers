// cmcstl2 - A concept-enabled C++ standard library
//
//  Copyright Casey Carter 2015 -- 2017
//  Copyright Eric Niebler 2015 -- 2017
//  Copyright Christopher Di Bella 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
#ifndef STL2_DETAIL_CONCEPTS_INSERTABLE_HPP
#define STL2_DETAIL_CONCEPTS_INSERTABLE_HPP

#include <stl2/detail/concepts/allocator.hpp>
#include <stl2/detail/concepts/core.hpp>
#include <stl2/memory.hpp>
#include <stl2/type_traits.hpp>
#include <stl2/utility.hpp>

STL2_OPEN_NAMESPACE {
   template <class T>
   struct __allocator_required {
      using type = allocator<value_type_t<T>>;
   };

   template <class T>
   requires
      requires() {
         typename T::allocator_type;
      }
   struct __allocator_required<T> {
      using type = allocator_t<T>;
   };

   template <class T>
   using __allocator_required_t = typename __allocator_required<T>::type;

   template <class X, class T, class A>
   concept bool __minimal_insertable = Same<value_type_t<X>, T>() &&
                                       Same<__allocator_required_t<X>, rebind_alloc_t<A, T>>() &&
                                       Allocator<A, T>();

   template <class X, class T, class A>
   concept bool DefaultInsertable() {
      return __minimal_insertable<X, T, A> &&
         requires(A m, T* p) {
            allocator_traits<A>::construct(m, p);
      };
   }

   template <class X, class T, class A>
   concept bool MoveInsertable() {
      return __minimal_insertable<X, T, A> &&
         requires(A m, T* p, T&& rv) {
            allocator_traits<A>::construct(m, p, rv);
      };
      // axiom: *p == previous_value(rv) && *p != rv && rv.~T()
   }

   template <class X, class T, class A>
   concept bool CopyInsertable() {
      return MoveInsertable<X, T, A>() &&
         requires(A m, T* p, const T& v) {
            allocator_traits<A>::construct(m, p, v);
      };
      // axiom: *p == v
   }

   template <class X, class T, class A, class... Args>
   concept bool EmplaceConstructible() {
      return __minimal_insertable<X, T, A> &&
         requires(A m, T* p, Args&&... args) {
            allocator_traits<A>::construct(m, p, __stl2::forward<Args>(args)...);
      };
   }

   template <class X, class T, class A>
   concept bool Erasable() {
      return __minimal_insertable<X, T, A> &&
         requires(A m, T* p) {
            allocator_traits<A>::destroy(m, p);
      };
   }
} STL2_CLOSE_NAMESPACE
#endif // STL2_DETAIL_CONCEPTS_INSERTABLE_HPP