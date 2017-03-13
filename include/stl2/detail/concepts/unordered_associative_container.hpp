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
#ifndef STL2_DETAIL_CONCEPTS_UNORDERED_ASSOCIATIVE_CONTAINER_HPP
#define STL2_DETAIL_CONCEPTS_UNORDERED_ASSOCIATIVE_CONTAINER_HPP

#include <stl2/detail/concepts/container.hpp>
#include <stl2/detail/concepts/associative_container.hpp>
#include <stl2/detail/concepts/meta.hpp>

STL2_OPEN_NAMESPACE {
   template <class X>
   using hasher_t = typename X::hasher;

   template <class X>
   using key_equal_t = typename X::key_equal;

   template <class X>
   using local_iterator_t = typename X::local_iterator;

   template <class X>
   using const_local_iterator_t = typename X::const_local_iterator;

   namespace __unordered_associative {
      template <class X>
      concept bool Set() {
         return Same<key_type_t<X>, value_type_t<X>>();
      }

      template <class X>
      concept bool Map() {
         return requires { typename mapped_type_t<X>; } &&
            Same<value_type_t<X>, pair<const key_type_t<X>, mapped_type_t<X>>>(); //TODO: replace with tagged_pair<tag::key(const key_type_t<X>), tag::value(mapped_type_t<X>)>>() &&
      }
   } // namespace __unordered_associative


   template <class X, class... Args>
   concept bool UnorderedAssociativeContainer() {
      return requires {
         typename key_type_t<X>;
         typename hasher_t<X>;
         typename key_equal_t<X>;
         typename local_iterator_t<X>;
         typename const_local_iterator_t<X>;
         // typename node_type_t<X>;
      } &&
         Invocable<hasher_t<X>, key_type_t<X>>() &&
         Relation<key_equal_t<X>, key_type_t<X>>() &&
         (__unordered_associative::Set<X>() || __unordered_associative::Map<X>()) &&
         DefaultConstructible<hasher_t<X>>() &&
         DefaultConstructible<key_equal_t<X>>() &&
         Container<X>() &&
         EmplaceConstructible<X, value_type_t<X>, allocator_t<X>, value_type_t<iterator_t<X>>>() &&
         EmplaceConstructible<X, value_type_t<X>, allocator_t<X>, Args...>() &&
         MoveInsertable<X, value_type_t<X>, allocator_t<X>>() &&
         CopyInsertable<X, value_type_t<X>, allocator_t<X>>() &&
         //Assignable<value_type_t<X>&, const value_type_t<X>&>() &&
         Constructible<X, size_type_t<X>, hasher_t<X>, key_equal_t<X>>() && // do we want this to be `const key_equal_t<X>&`?
         Constructible<X, size_type_t<X>, hasher_t<X>>() &&
         Constructible<X, size_type_t<X>>() &&
         Constructible<X, iterator_t<X>, sentinel_t<X>, size_type_t<X>, hasher_t<X>, key_equal_t<X>>() &&
         Constructible<X, iterator_t<X>, sentinel_t<X>, size_type_t<X>, hasher_t<X>>() &&
         Constructible<X, iterator_t<X>, sentinel_t<X>, size_type_t<X>>() &&
         Constructible<X, iterator_t<X>, sentinel_t<X>>() &&
         Constructible<X, initializer_list<value_type_t<X>>>() &&
         Constructible<X, initializer_list<value_type_t<X>>, size_type_t<X>>() &&
         Constructible<X, initializer_list<value_type_t<X>>, size_type_t<X>, hasher_t<X>>() &&
         Constructible<X, initializer_list<value_type_t<X>>, size_type_t<X>, hasher_t<X>, key_equal_t<X>>() &&
         Assignable<X&, initializer_list<value_type_t<X>>>() &&
         // TODO node stuff (a2 should be node_type_t<X>)... C++17 related?
         requires(X a, X a2, const X b, iterator_t<X> i, sentinel_t<X> j, const_iterator_t<X> p,
            const_sentinel_t<X> q2, const_sentinel_t<X> q, const_iterator_t<X> q1, iterator_t<X> r,
            initializer_list<value_type_t<X>> il, value_type_t<X> t, key_type_t<X> k,
            hasher_t<X> hf, key_equal_t<X> eq, size_type_t<X> n, float z) {
            {b.hash_function()} -> hasher_t<X>;
            {b.key_eq()} -> key_equal_t<X>;
            //{a.emplace_hint(p, __stl2::forward<Args>(args)...)} -> iterator_t<X>;
            {a.insert(p, t)} -> iterator_t<X>;
            {a.insert(i, j)} -> void;
            {a.insert(il)} -> void;
            //{a.insert(q, nh)} -> iterator_t<X>;
            //{a.extract(k)} -> node_type_t<X>;
            //{a.extract(q)} -> node_type_t<X>;
            //{a.merge(a2)} -> void;
            {a.erase(k)} -> size_type_t<X>;
            {a.erase(q)} -> iterator_t<X>;
            {a.erase(r)} -> iterator_t<X>;
            {a.erase(q1, q2)} -> iterator_t<X>;
            {a.clear()} noexcept -> void;
            {a.find(k)} -> iterator_t<X>;
            {b.find(k)} -> const_iterator_t<X>;
            {b.count(k)} -> size_type_t<X>;
            {a.equal_range(k)} -> pair<iterator_t<X>, iterator_t<X>>; // TODO replace with tagged_pair<tag::first(iterator_t<X>), tag::last(iterator_t<X>)?
            {b.equal_range(k)} -> pair<const_iterator_t<X>, const_iterator_t<X>>; // TODO replace with tagged_pair<tag::first(const_iterator_t<X>), tag::last(iterator_t<X>)>
            {b.bucket_count()} -> size_type_t<X>;
            {b.max_bucket_count()} -> size_type_t<X>;
            {b.bucket(k)} -> size_type_t<X>;
            {b.bucket_size(n)} -> size_type_t<X>;
            {a.begin(n)} -> local_iterator_t<X>;
            {b.begin(n)} -> const_local_iterator_t<X>;
            {a.end(n)} -> local_iterator_t<X>;
            {b.end(n)} -> const_local_iterator_t<X>;
            {b.cbegin(n)} -> const_local_iterator_t<X>;
            {b.cend(n)} -> const_local_iterator_t<X>;
            {b.load_factor()} -> float; // TODO replace with double?
            {b.max_load_factor()} -> float; // TODO replace with double?
            {a.max_load_factor(z)} -> void;
            {a.rehash(n)} -> void;
            {a.reserve(n)} -> void;
         } &&
         (__KeyUnique<X, Args...>() || __KeyEqual<X, Args...>());
   }
} STL2_CLOSE_NAMESPACE

#endif // STL2_DETAIL_CONCEPTS_UNORDERED_ASSOCIATIVE_CONTAINER_HPP