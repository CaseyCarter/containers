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
#ifndef STL2_DETAIL_CONCEPTS_SEQUENCE_CONTAINER_HPP
#define STL2_DETAIL_CONCEPTS_SEQUENCE_CONTAINER_HPP

#include <stl2/detail/concepts/container.hpp>
#include <deque>
#include <initializer_list>
#include <stl2/detail/concepts/fundamental.hpp>
#include <stl2/detail/concepts/insertable.hpp>
#include <stl2/detail/concepts/meta.hpp>
#include <stl2/detail/concepts/object.hpp>
#include <stl2/detail/fwd.hpp>
#include <stl2/iterator.hpp>
#include <vector>

STL2_OPEN_NAMESPACE {
   template <class X, class T, class A>
   constexpr bool __is_vector_or_deque = meta::_Or<Same<vector<T, A>, X>(), Same<deque<T, A>, X>()>;

   template <class X, class T, class A>
   concept bool __is_vector_or_deque_and_also_assignable =
      meta::_Xor<__is_vector_or_deque<X, T, A> && Assignable<T&, T>(), Assignable<T&, T>()>;

   namespace opt {
// very icky... should I write a proposal for an "optional" requirement?
// e.g.
//
// requires(X a) {
//    optional {a.pop_back()} -> void;
// }
//
// "optional" would be a context-sensitive keyword, so no clashing with std::optional :)
// equivalent to the macro directly below, but with proper language support
#define OPTIONAL_REQUIRES(EX, ARR, TEE) (requires EX { {ARR} -> TEE; } || not requires EX { {ARR}; })
      template <class X, class... Args>
      concept bool front =  OPTIONAL_REQUIRES((X a), a.front(), value_type_t<X>&)
                         && OPTIONAL_REQUIRES((const X c), c.front(), const value_type_t<X>&)
                       //  && OPTIONAL_REQUIRES((X a, Args&&... args), a.emplace_front(std::forward<Args>(args)...), value_type_t<X>&)
                         && OPTIONAL_REQUIRES((X a, value_type_t<X> t), a.push_front(t), void)
                         && OPTIONAL_REQUIRES((X a, value_type_t<X>&& rv), a.push_front(rv), void)
                         && OPTIONAL_REQUIRES((X a), a.pop_front(), void)
                         ;

      template <class X, class... Args>
      concept bool back =  OPTIONAL_REQUIRES((X a), a.back(), value_type_t<X>&)
                        && OPTIONAL_REQUIRES((const X c), c.back(), const value_type_t<X>&)
                   //     && OPTIONAL_REQUIRES((X a, Args&&... args), a.emplace_back(std::forward<Args>(args)...), value_type_t<X>&)
                        && OPTIONAL_REQUIRES((X a, value_type_t<X> t), a.push_back(t), void)
                        && OPTIONAL_REQUIRES((X a, value_type_t<X>&& rv), a.push_back(rv), void)
                        && OPTIONAL_REQUIRES((X a), a.pop_back(), void)
                        ;

      template <class X>
      concept bool subscript = OPTIONAL_REQUIRES((X a, size_type_t<X> n), a[n], value_type_t<X>&)
                            && OPTIONAL_REQUIRES((const X a, size_type_t<X> n), a[n], const value_type_t<X>&)
                            && OPTIONAL_REQUIRES((X a, size_type_t<X> n), a.at(n), value_type_t<X>&)
                            && OPTIONAL_REQUIRES((const X a, size_type_t<X> n), a.at(n), const value_type_t<X>&)
                            ;
#undef OPTIONAL_REQUIRES
   } // namespace opt

   template <class X, class T, class A, class... Args>
   concept bool __SequenceAllocatable() {
      return Constructible<X, size_type_t<X>, T>() && // axiom: count(a, t) == n;
         Constructible<X, initializer_list<T>>() && // axiom: ranges::equal(a, il.begin());
         Assignable<X&, initializer_list<T>>() &&
         requires(X a, initializer_list<T> il, size_type_t<X> n,
                  const_iterator_t<X> p, T t, T&& rv, Args&&... args) {
            {a.emplace(p, args...)} -> iterator_t<X>;
            {a.insert(p, t)} -> iterator_t<X>;
            {a.insert(p, rv)} -> iterator_t<X>;
            {a.insert(p, n, t)} -> iterator_t<X>;
            {a.insert(p, il)} -> iterator_t<X>;
            __is_vector_or_deque_and_also_assignable<X, T, A>;
            {a.erase(p)} -> iterator_t<X>;
            {a.erase(p, p)} -> iterator_t<X>;
            {a.clear()} -> void;
            {a.assign(il)} -> void;
            {a.assign(n, t)} -> void;
      } &&
      opt::front<X> &&
      opt::back<X> &&
      opt::subscript<X>;
   }

   template <class X, class T, class A, class I, class S, class... Args>
   concept bool __SequenceContainerExtraTraits() {
      return Assignable<value_type_t<X>&, value_type_t<I>>() &&
         EmplaceConstructible<X, T, A, value_type_t<I>>() &&
         Constructible<X, I, S>() && // axiom: ranges::distance(i, j) == ranges::distance(a.begin(), a.end()) && ranges::equal(a, i);
         requires(X a, const_iterator_t<X> p, I i, S j) {
            {a.insert(p, i, j)} -> iterator_t<X>;
            {a.assign(i, j)};
         };
   }

   template <class X, class... Args>
   concept bool SequenceContainer() {
      return Container<X>() &&
         __SequenceAllocatable<X, value_type_t<X>, __allocator_required_t<X>, Args...>() &&
         __SequenceContainerExtraTraits<X, value_type_t<X>, __allocator_required_t<X>, iterator_t<X>, sentinel_t<X>>();
   }

   template <class X, class I, class S, class... Args>
   concept bool SequenceContainer() {
      return SequenceContainer<X, Args...>() &&
         InputIterator<I>() &&
         Sentinel<S, I>() &&
         __SequenceContainerExtraTraits<X, value_type_t<X>, __allocator_required_t<X>, I, S>();
   }
} STL2_CLOSE_NAMESPACE

#endif // STL2_DETAIL_CONCEPTS_SEQUENCE_CONTAINER_HPP
