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
#ifndef STL2_DETAIL_CONCEPTS_META_HPP
#define STL2_DETAIL_CONCEPTS_META_HPP

#include <stl2/detail/fwd.hpp>

STL2_OPEN_NAMESPACE {
   namespace meta {
      template <bool A, bool... Args>
      constexpr bool _Or = A || _Or<Args...>;

      template <bool A>
      constexpr bool _Or<A> = A;

      template <bool A, bool... Args>
      constexpr bool _Xor = A ^ _Xor<Args...>;

      template <bool A>
      constexpr bool _Xor<A> = A;
   } // namespace meta
} STL2_CLOSE_NAMESPACE
#endif // STL2_DETAIL_CONCEPTS_META_HPP