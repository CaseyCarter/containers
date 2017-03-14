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
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "validate.hpp"
#include <vector>

namespace ranges = __stl2;
using ranges::ContiguousContainer;

CONCEPT_ASSERT(ContiguousContainer<std::array<int, 10>>());
CONCEPT_ASSERT(ContiguousContainer<std::deque<int>>());
CONCEPT_ASSERT(!ContiguousContainer<std::forward_list<int>>());
CONCEPT_ASSERT(!ContiguousContainer<std::list<int>>());
CONCEPT_ASSERT(!ContiguousContainer<std::map<int, int>>());
CONCEPT_ASSERT(!ContiguousContainer<std::multimap<int, int>>());
CONCEPT_ASSERT(!ContiguousContainer<std::set<int>>());
CONCEPT_ASSERT(!ContiguousContainer<std::multiset<int>>());
CONCEPT_ASSERT(!ContiguousContainer<std::unordered_map<int, int>>());
CONCEPT_ASSERT(!ContiguousContainer<std::unordered_multimap<int, int>>());
CONCEPT_ASSERT(!ContiguousContainer<std::unordered_set<int>>());
CONCEPT_ASSERT(!ContiguousContainer<std::unordered_multiset<int>>());
CONCEPT_ASSERT(ContiguousContainer<std::vector<int>>());

using Fool_type = ranges::size_type_t<std::set<int>>;
CONCEPT_ASSERT(!ContiguousContainer<std::set<Fool_type>>());
//CONCEPT_ASSERT(!ContiguousContainer<std::multiset<Fool_type>>());
//CONCEPT_ASSERT(!ContiguousContainer<std::unordered_set<Fool_type>>());
//CONCEPT_ASSERT(!ContiguousContainer<std::unordered_multiset<Fool_type>>());
