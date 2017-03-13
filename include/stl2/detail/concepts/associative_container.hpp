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
#ifndef STL2_DETAIL_CONCEPTS_ASSOCIATIVE_CONTAINER_HPP
#define STL2_DETAIL_CONCEPTS_ASSOCIATIVE_CONTAINER_HPP

#include <stl2/detail/concepts/container.hpp>
#include <stl2/detail/concepts/meta.hpp>

STL2_OPEN_NAMESPACE {
   template <class X>
   using key_type_t = typename X::key_type;

   template <class X>
   using key_compare_t = typename X::key_compare;

   template <class X>
   using value_compare_t = typename X::value_compare;

   template <class X>
   using mapped_type_t = typename X::mapped_type;

   template <class X>
   using node_type_t = typename X::node_type;

   namespace __associative {
      template <class X>
      concept bool Set() {
         return Same<key_type_t<X>, value_type_t<X>>() &&
            Same<key_compare_t<X>, value_compare_t<X>>() &&
            Same<iterator_t<X>, const_iterator_t<X>>();
      }

      template <class X>
      concept bool Map() {
         return requires { typename mapped_type_t<X>; } &&
            Same<value_type_t<X>, pair<const key_type_t<X>, mapped_type_t<X>>>(); //TODO: replace with tagged_pair<tag::key(const key_type_t<X>), tag::value(mapped_type_t<X>)>>() &&
            // TODO: potentially something about value_compare_t<X> here
      }
   } // namespace __associative

   template <class X, class... Args>
   concept bool __KeyUnique() { // exposition only
      return requires(X a, value_type_t<X> t) {
         // {a.emplace(__stl2::forward<Args>(args)...)} -> optional<iterator_t<X>>;
         // axiom: ???
         // complexity: ???

         {a.insert(t)} -> std::pair<iterator_t<X>, bool>; // todo: swap with optional<iterator_t<X>>
         // axiom: ???
         // complexity: ???

         //{a.insert(nh)} -> insert_return_type;
         // axiom: ???
         // complexity: ???
      };
   }

   template <class X, class... Args>
   concept bool __KeyEqual() { // exposition only
      return requires(X a, value_type_t<X> t) {
         // {a.emplace(__stl2::forward<Args>(args)...)} -> iterator_t<X>;
         // axiom: ???
         // complexity: ???

         {a.insert(t)} -> iterator_t<X>;
         // axiom: ???
         // complexity: ???

         //{a.insert(nh)} -> iterator_t<X>;
         // axiom: ???
         // complexity: ???
      };
   }

   template <class X, class... Args>
   concept bool AssociativeContainer() {
      return requires {
            typename key_type_t<X>;
            typename key_compare_t<X>;
            typename value_compare_t<X>;
            //typename node_type_t<X>;
         } &&
         CopyConstructible<key_compare_t<X>>() &&
         Relation<value_compare_t<X>, value_type_t<X>>() &&
         (__associative::Set<X>() || __associative::Map<X>()) &&
         DefaultConstructible<key_compare_t<X>>() &&
         Container<X>() &&
         // TODO: requires node_handle
         Constructible<X, key_compare_t<X>>() &&
         Constructible<X, initializer_list<value_type_t<X>>>() &&
         Constructible<X, initializer_list<value_type_t<X>>, key_compare_t<X>>() &&
         requires(X a, X a2, const X b, value_type_t<X> t, key_type_t<X> k,
            iterator_t<X> r, const_iterator_t<X> p, const_iterator_t<X> q,
            iterator_t<X> i, sentinel_t<X> j, const_iterator_t<X> q1, const_sentinel_t<X> q2,
            initializer_list<value_type_t<X>> il) {
            // axiom:
            //    return valid_iterator(a, r);
            // axiom:
            //    return valid_iterator(a, p);
            // axiom:
            //    return valid_iterator(a, q) && q != a.end();
            // axiom:
            //    return valid_range(a, i, j);
            // axiom:
            //    return valid_range(a, q1, q2);

            //{a.emplace_hint(p, __stl2::forward<Args>(args)...)} -> iterator_t<X>;
            // axiom: ???
            // complexity: ???

            {a.insert(p, t)} -> iterator_t<X>;
            // axiom: ???
            // complexity: ???

            {a.insert(i, j)} -> void;
            // expects:
            //    !is_iterator_to(a, i);
            //    !is_sentinel_to(a, j);
            // axiom: ???
            // complexity: ???

            {a.insert(il)} -> void;
            // axiom:
            //    return a.insert(il) <==> a.insert(il.begin(), il.end());
            // complexity: ???

            //{a.insert(p, nh)} -> iterator_t<X>;
            // axiom:
            // complexity:

            //{a.extract(k)} -> node_type_t<X>;
            // axiom:
            // complexity:

            //{a.extract(p)} -> node_type_t<X>;
            // axiom:
            // complexity:

            //{a.merge(a2)} -> void;
            // axiom:
            // complexity: ???

            {a.erase(k)} -> size_type_t<X>;
            // axiom:
            //    size_type_t<X> n = a.count(k);
            //    return a.erase(k) == n;
            // ensures:
            //    retrun a.find(k) == a.end();
            // complexity: ???

            {a.erase(q)} -> iterator_t<X>;
            // axiom {__ordered::Is_set}:
            //    auto x = *q;
            //    q = a.erase(q);
            //    return q == a.end() || x.first <= q->first>
            // axiom {__ordered::Is_map}:
            //    auto x = *q;
            //    q = a.erase(q);
            //    return q == a.end() || (x.first == q->first && x.second != q->second) || (x.first < q->first)>
            // ensures:
            //    *r removed from container
            // complexity: ???

            {a.erase(r)} -> iterator_t<X>;
            // axiom {__ordered::Is_set}:
            //    auto x = *r;
            //    r = a.erase(r);
            //    return r == a.end() || x.first <= r->first>
            // axiom {__ordered::Is_map}:
            //    auto x = *r;
            //    r = a.erase(r);
            //    return r == a.end() || (x.first == r->first && x.second != r->second) || (x.first < r->first)>
            // ensures:
            //    * r removed from container
            // complexity: ???

            {a.erase(q1, q2)} -> iterator_t<X>;
            // axiom:
            //    r = a.erase(q1, q2);
            //    r == a.end() || r == q2;
            // ensures:
            //    range [q1, q2) removed from container
            // complexity: ???

            {a.clear()} noexcept -> void;
            // axiom:
            //    a.erase(a.begin(), a.end());
            // ensures:
            //    return a.empty();
            // complexity: ???

            {a.find(k)} -> iterator_t<X>;
            {b.find(k)} -> const_iterator_t<X>;
            // axiom:
            //    r = a.find(k);
            //    return r == a.end() || r->first = k;
            // axiom:
            //    return a.find(k) == __stl2::find_if(a, [&k](const auto& i){ return i.first == k; });
            // complexity: ???

            {b.count(k)} -> size_type_t<X>;
            // axiom:
            //    return b.count(k) == __stl2::count(b, k);
            // complexity: ???

            {a.lower_bound(k)} -> iterator_t<X>;
            {b.lower_bound(k)} -> const_iterator_t<X>;
            // axiom:
            //    r = a.lower_bound(k);
            //    return i == a.end() || k < *i;
            // complexity: ???

            {a.upper_bound(k)} -> iterator_t<X>;
            {b.upper_bound(k)} -> const_iterator_t<X>;
            // axiom:
            //    r = a.upper_bound(k);
            //    return i == a.end() || *i > k;
            // complexity: ???

            {a.equal_range(k)} -> tagged_pair<tag::begin(iterator_t<X>), tag::end(iterator_t<X>)>;
            {b.equal_range(k)} -> tagged_pair<tag::begin(const_iterator_t<X>), tag::end(const_iterator_t<X>)>;
            // axiom:
            //    return a.equal_range(k) == __stl2::make_tagged_pair<tag::begin, tag::end>(a.lower_bound(k), a.upper_bound(k));
            // complexity: ???
         } &&
         (__KeyUnique<X, Args...>() || __KeyEqual<X, Args...>());
   }

   template <class X>
   concept bool TransparentAssociativeContainer() {
      return AssociativeContainer<X>() &&
         requires(X a, const X b, key_compare_t<X> c, iterator_t<X> r, value_type_t<X> kl, value_type_t<X> ku, value_type_t<X> ke) {
            // axiom: r is valid in a ???
            // axiom: return r != a.end();
            // axiom: c(r, kl) => with r the key value in e and e in a ??? wtf does this even mean
            // axiom: return !c(ku, r);
            // axiom: return c(r, ke) <=> !c(ke, r);
            typename key_compare_t<X>::is_transparent;

            {a.find(ke)} -> iterator_t<X>;
            {b.find(ke)} -> const_iterator_t<X>;
            // axiom: r = a.find(ke); return !c(r, ke) && !c(ke, r);
            // complexity: ???

            {a.count(ke)} -> size_type_t<X>;
            // axiom: ???
            // complexity: ???

            {a.lower_bound(ke)} -> iterator_t<X>;
            {b.lower_bound(ke)} -> const_iterator_t<X>;
            // axiom: r = a.lower_bound(k); return i == a.end() || k < *i;
            // complexity: ???

            {a.upper_bound(ke)} -> iterator_t<X>;
            {b.upper_bound(ke)} -> const_iterator_t<X>;
            // axiom: r = a.upper_bound(k); return i == a.end() || *i > k;
            // complexity: ???

            {a.equal_range(ke)} -> tagged_pair<tag::begin(iterator_t<X>), tag::end(iterator_t<X>)>;
            {b.equal_range(ke)} -> tagged_pair<tag::begin(const_iterator_t<X>), tag::end(const_iterator_t<X>)>;
            // axiom: return a.equal_range(k) == __stl2::make_tagged_pair<tag::begin, tag::end>(a.lower_bound(k), a.upper_bound(k));
            // complexity: ???
         };
   }

   template <class X, class I, class S>
   concept bool AssociativeContainer() {
      return AssociativeContainer<X>() &&
         InputIterator<I>() &&
         Sentinel<S, I>() &&
         Assignable<value_type_t<X>&, value_type_t<I>>() &&
         EmplaceConstructible<X, value_type_t<X>, allocator_t<X>, value_type_t<I>>() &&
         Constructible<X, I, S>() &&
         Constructible<X, I, S, key_compare_t<X>>() &&
         requires(X a, I i, S j) {
            {a.insert(i, j)} -> void;
            // expects:
            //    !is_iterator_to(a, i);
            //    !is_sentinel_to(a, j);
            // axiom: ???
            // complexity: ???
         };
   }
} STL2_CLOSE_NAMESPACE

#endif // STL2_DETAIL_CONCEPTS_ASSOCIATIVE_CONTAINER_HPP