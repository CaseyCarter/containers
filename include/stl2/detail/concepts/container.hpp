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
#ifndef STL2_DETAIL_CONCEPTS_CONTAINER_HPP
#define STL2_DETAIL_CONCEPTS_CONTAINER_HPP

#include <initializer_list>
#include <stl2/detail/concepts/fundamental.hpp>
#include <stl2/detail/concepts/object.hpp>
#include <stl2/detail/concepts/insertable.hpp>
#include <stl2/detail/fwd.hpp>
#include <stl2/iterator.hpp>

STL2_OPEN_NAMESPACE {
   template <class T>
   using const_iterator_t = iterator_t<const T>;

   template <class T>
   using const_sentinel_t = sentinel_t<const T>;

   template <class T>
   using size_type_t = typename T::size_type;

   template <class X>
   concept bool __ContainerAliases() {
      return requires(X) {
         typename value_type_t<X>;
         typename X::reference;
         typename X::const_reference;
         typename difference_type_t<X>;
         typename size_type_t<X>;
      } &&
      models::Same<typename X::reference, value_type_t<X>&> &&
      models::Same<typename X::const_reference, const value_type_t<X>&> &&
      models::UnsignedIntegral<size_type_t<X>> && 
      models::SignedIntegral<difference_type_t<X>> &&
      sizeof(size_type_t<X>) >= sizeof(difference_type_t<X>);
   }

   template <class X>
   concept bool __ContainerIterators() {
      return models::ForwardRange<X> &&
         requires(X a, const X& b) {
            typename iterator_t<X>;
            typename sentinel_t<X>;
            typename const_iterator_t<X>;
            typename const_sentinel_t<X>;
      } &&
         models::Constructible<const_iterator_t<X>, iterator_t<X>> &&
         models::Same<value_type_t<iterator_t<X>>, value_type_t<X>> &&
         requires(X a, const X b) {
            {a.begin()} -> iterator_t<X>;
            {a.end()} -> sentinel_t<X>;
            {a.cbegin()} -> const_iterator_t<X>;
            {a.cend()} -> const_sentinel_t<X>;
            {b.begin()} -> const_iterator_t<X>;
            {b.end()} -> const_sentinel_t<X>;
      };
   }

   // stricter than a C++17 Container since this requires
   // both MoveAssignable _and_ CopyAssignable be true
   template <class X>
   concept bool Container() {
      return InputRange<X>() &&
         EqualityComparable<X>() &&
         __ContainerAliases<X>() &&
         __ContainerIterators<X>() &&
         CopyInsertable<X, value_type_t<X>, __allocator_required_t<X>>() &&
         Erasable<X, value_type_t<X>, __allocator_required_t<X>>() &&
         Swappable<X&>() &&
         requires(X a, X b, const X& c) {
            {a.swap(b)}; // TODO: specify a noexcept specifier that captures _all_ C++17 containers, including array
            {a.size()} noexcept -> size_type_t<X>;       // TODO: debate size_type_t vs
            {a.max_size()} noexcept -> size_type_t<X>;   // difference_type_t. See stl2/issues/#xxx
            {a.empty()} noexcept -> Boolean;
      };
      // axiom: a.swap(b) is equivalent to swap(a, b)
   }

   template <class X, class T, class A, class... Args>
   concept bool __SequenceAllocatable() {
      return
         requires(X a, initializer_list<T> il, size_type_t<X> n,
                  const_iterator_t<X> p, const_iterator_t<X> q,
                  T t, T&& rv, Args&&... args) {
         {X(n, t)}; // axiom: count(a, t) == n;
         requires EmplaceConstructible<X, T, A, value_type_t<decltype(*i)>>;
         {X(il)}; // axiom: ranges::equal(a, il.begin());
         //{}
      };
   }

   template <class X>
   concept bool SequenceContainer() {
      return Container<X>() &&
         __SequenceAllocatable<X, value_type_t<X>, __allocator_required_t<X>>();
   }

   template <class X, class I>
   concept bool SequenceContainer() {
      return SequenceContainer<X>() &&
         Iterator<I>() &&
         requires(I i, I j, value_type<X> t) {
            {t = *i} -> value_type<X>&;
            {X(i, j)}; // axiom: ranges::distance(i, j) == ranges::distance(a.begin(), a.end()) && ranges::equal(a, i);
      };
   }
} STL2_CLOSE_NAMESPACE
#endif // STL2_DETAIL_CONCEPTS_CONTAINER_HPP
