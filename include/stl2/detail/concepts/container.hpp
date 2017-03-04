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
   using reverse_iterator_t =  decltype(__stl2::rbegin(declval<T&>()));

   template <class T>
   using reverse_sentinel_t =  decltype(__stl2::rend(declval<T&>()));

   template <class T>
   using const_reverse_iterator_t = reverse_iterator_t<const T>;

   template <class T>
   using const_reverse_sentinel_t = reverse_sentinel_t<const T>;

   template <class T>
   using const_sentinel_t = sentinel_t<const T>;

   template <class T>
   using size_type_t = typename T::size_type;

   template <class X>
   concept bool __ContainerAliases() {
      return requires {
         typename value_type_t<X>;
         typename X::reference;
         typename X::const_reference;
         typename difference_type_t<X>;
         typename size_type_t<X>;
      } &&
      Same<typename X::reference, value_type_t<X>&>() &&
      Same<typename X::const_reference, const value_type_t<X>&>() &&
      UnsignedIntegral<size_type_t<X>>() && 
      SignedIntegral<difference_type_t<X>>() &&
      sizeof(size_type_t<X>) >= sizeof(difference_type_t<X>);
   }

   template <class X>
   concept bool __ContainerIterators() {
      return ForwardRange<X>() &&
         requires {
            typename iterator_t<X>;
            typename sentinel_t<X>;
            typename const_iterator_t<X>;
            typename const_sentinel_t<X>;
         } &&
         Constructible<const_iterator_t<X>, iterator_t<X>>() &&
         Same<value_type_t<iterator_t<X>>, value_type_t<X>>() &&
         requires(X a, const X b, iterator_t<X> i) {
            {a.begin()} -> iterator_t<X>;
            {a.end()} -> sentinel_t<X>;
            {a.cbegin()} -> const_iterator_t<X>;
            {a.cend()} -> const_sentinel_t<X>;
            {b.begin()} -> const_iterator_t<X>;
            {b.end()} -> const_sentinel_t<X>;
            //{i - i} -> Same<difference_type_t<X>>;
      };
   }

   // stricter than a C++17 Container since this requires
   // both MoveAssignable _and_ CopyAssignable be true
   template <class X>
   concept bool Container() {
      return EqualityComparable<value_type_t<X>>() &&
         CopyInsertable<X, value_type_t<X>, __allocator_required_t<X>>() &&
        // Regular<X>() &&
         __ContainerAliases<X>() &&
         __ContainerIterators<X>() &&
         Erasable<X, value_type_t<X>, __allocator_required_t<X>>() &&
         requires(X a, X b, const X& c) {
            {a.swap(b)}; // TODO: specify a noexcept specifier that captures _all_ C++17 containers, including array
            {a.size()} noexcept -> size_type_t<X>;       // TODO: debate size_type_t vs
            {a.max_size()} noexcept -> size_type_t<X>;   // difference_type_t. See stl2/issues/#xxx
            {a.empty()} noexcept -> Boolean;
      };
      // axiom: a.swap(b) is equivalent to swap(a, b)
   }

   template <class X>
   concept bool OrderedContainer() {
      return Container<X>() &&
         StrictTotallyOrdered<value_type_t<X>>() &&
         StrictTotallyOrdered<X>();
   }

   template <class X>
   concept bool ReversibleContainer() {
      return Container<X>() &&
         BidirectionalRange<X>() &&
         requires(X x, const X cx) {
            typename reverse_iterator_t<X>;
            typename reverse_sentinel_t<X>;
            typename const_reverse_iterator_t<X>;
            typename const_reverse_sentinel_t<X>;
            {x.rbegin()} -> reverse_iterator_t<X>;
            {x.rend()} -> reverse_sentinel_t<X>;
            {x.crbegin()} -> const_reverse_iterator_t<X>;
            {x.crend()} -> const_reverse_sentinel_t<X>;
            {cx.rbegin()} -> const_reverse_iterator_t<X>;
            {cx.rend()} -> const_reverse_sentinel_t<X>;
         };
   }

   template <class X>
   concept bool ReversibleOrderedContainer() {
      return OrderedContainer<X>() &&
         ReversibleContainer<X>();
   }

   // note I am refusing to make AllocatorAwareOrderedContainer,
   // AllocatorAwareReversibleContainer, and AllocatorAwareReversibleOrderedContainer.
   // Users can type out the Ordered/Reversible requirements if they need that aspect
   template <class X, class T, class A>
   concept bool __AllocatorAwareContainerTraits() {
      return Same<value_type_t<A>, T>() &&
         DefaultConstructible<A>() &&
         DefaultConstructible<X>() &&
         Constructible<X, A>() &&
         //Assignable<T&, const T&>() &&        doesn't permit [unordered_]map
         Constructible<X, const X&, A>() &&
         MoveInsertable<X, T, A>() &&
         //Assignable<T&, T>() &&               ditto
         Constructible<X, X&&, A>();
   }

   template <class X>
   concept bool AllocatorAwareContainer() {
      return Container<X>() &&
         requires {typename allocator_t<X>;} &&
         __AllocatorAwareContainerTraits<X, value_type_t<X>, allocator_t<X>>();
   }

   // designed to cause a compile-time error when a real ContiguousIterator is created
   // that way, the ContiguousContainer below is always correctly defined
   template <class X>
   concept bool ContiguousIterator() {
      return RandomAccessIterator<X>();
   }

   template <class X>
   concept bool ContiguousRange() {
      return RandomAccessRange<X>() &&
         ContiguousIterator<iterator_t<X>>();
   }

   template <class X>
   concept bool ContiguousContainer() {
      return Container<X>() &&
         ContiguousRange<X>() &&
         requires(X a, const X c, size_type_t<X> n) {
            {a[n]} -> value_type_t<X>&;
            {a[n]} -> const value_type_t<X>&;
            {a.at(n)} -> value_type_t<X>&;
            {a.at(n)} -> const value_type_t<X>&;
      };
   }
} STL2_CLOSE_NAMESPACE
#endif // STL2_DETAIL_CONCEPTS_CONTAINER_HPP
