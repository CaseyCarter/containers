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
#include <stl2/detail/concepts/sequence_container.hpp>
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include "validate.hpp"
#include <vector>

namespace ranges = __stl2;
using ranges::SequenceContainer;

CONCEPT_ASSERT(!SequenceContainer<std::array<int, 10>>());
CONCEPT_ASSERT(ranges::SequenceContainer<std::deque<int>>());
CONCEPT_ASSERT(!SequenceContainer<std::forward_list<int>>());
CONCEPT_ASSERT(SequenceContainer<std::list<int>>());
CONCEPT_ASSERT(!SequenceContainer<std::map<int, int>>());
CONCEPT_ASSERT(!SequenceContainer<std::set<int>>());
CONCEPT_ASSERT(!SequenceContainer<std::unordered_map<int, int>>());
CONCEPT_ASSERT(!SequenceContainer<std::unordered_set<int>>());
CONCEPT_ASSERT(ranges::SequenceContainer<std::vector<int>>());

CONCEPT_ASSERT(!ranges::FrontInsertable<std::array<int, 10>>());
CONCEPT_ASSERT(ranges::FrontInsertable<std::forward_list<int>>());
CONCEPT_ASSERT(ranges::FrontInsertable<std::deque<int>>());
CONCEPT_ASSERT(ranges::FrontInsertable<std::list<int>>());
CONCEPT_ASSERT(!ranges::FrontInsertable<std::map<int, int>>());
CONCEPT_ASSERT(!ranges::FrontInsertable<std::set<int>>());
CONCEPT_ASSERT(!ranges::FrontInsertable<std::unordered_map<int, int>>());
CONCEPT_ASSERT(!ranges::FrontInsertable<std::unordered_set<int>>());
CONCEPT_ASSERT(!ranges::FrontInsertable<std::vector<int>>());
