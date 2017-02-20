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
#include <stl2/detail/concepts/container.hpp>
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <stl2/utility.hpp>
#include "validate.hpp"
#include <vector>

namespace ranges = __stl2;
using ranges::Container;

namespace std {
   template <class T>
   requires
      !ranges::Movable<T>()
   struct hash<T> {
      using argument_type = T;
      using result_type = size_t;
      result_type operator()(const argument_type&) const
      {
         return 0;
      }
   };
}

///////////////////// with std::allocator and non-movable types /////////////////////
// sequence containers
CONCEPT_ASSERT(Container<std::deque<std::mutex>>());
CONCEPT_ASSERT(!Container<std::forward_list<std::random_device>>());
CONCEPT_ASSERT(Container<std::list<std::mutex>>());
CONCEPT_ASSERT(Container<std::vector<std::random_device>>());

// associative containers
CONCEPT_ASSERT(Container<std::map<std::mutex, std::random_device>>());
CONCEPT_ASSERT(Container<std::set<std::random_device>>());
CONCEPT_ASSERT(Container<std::unordered_map<std::random_device, std::mutex>>());
CONCEPT_ASSERT(Container<std::unordered_set<std::mutex>>());

// Bjarne's "almost containers"
CONCEPT_ASSERT(Container<std::array<std::random_device, 10>>()); // I think this one is okay to pass
