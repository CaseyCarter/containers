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
#include <stl2/detail/concepts/unordered_associative_container.hpp>
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <stl2/functional.hpp>
#include <unordered_map>
#include <unordered_set>
#include "validate.hpp"
#include <vector>

namespace ranges = __stl2;
using ranges::UnorderedAssociativeContainer;

CONCEPT_ASSERT(!UnorderedAssociativeContainer<std::array<int, 10>>());
CONCEPT_ASSERT(!UnorderedAssociativeContainer<std::deque<int>>());
CONCEPT_ASSERT(!UnorderedAssociativeContainer<std::forward_list<int>>());
CONCEPT_ASSERT(!UnorderedAssociativeContainer<std::list<int>>());
CONCEPT_ASSERT(!UnorderedAssociativeContainer<std::map<int, int>>());
CONCEPT_ASSERT(!UnorderedAssociativeContainer<std::map<int, int, ranges::greater<int>>>());
CONCEPT_ASSERT(!UnorderedAssociativeContainer<std::multimap<int, int>>());
//CONCEPT_ASSERT(
CONCEPT_ASSERT(!UnorderedAssociativeContainer<std::set<int>>());
CONCEPT_ASSERT(!UnorderedAssociativeContainer<std::multiset<int>>());
CONCEPT_ASSERT(UnorderedAssociativeContainer<std::unordered_map<int, int>>());
CONCEPT_ASSERT(UnorderedAssociativeContainer<std::unordered_multimap<int, int>>());
CONCEPT_ASSERT(UnorderedAssociativeContainer<std::unordered_set<int>>());
CONCEPT_ASSERT(UnorderedAssociativeContainer<std::unordered_multiset<int>>());
CONCEPT_ASSERT(!UnorderedAssociativeContainer<std::vector<int>>());

ranges::UnorderedAssociativeContainer foo = std::unordered_set<int>{};
