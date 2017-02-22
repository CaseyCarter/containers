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
#include "../../cmcstl2/test/simple_test.hpp"
#include <array>
#include <bitset>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <stl2/utility.hpp>
#include <valarray>
#include "validate.hpp"
#include <vector>

namespace ranges = __stl2;
using ranges::Container;
using ranges::OrderedContainer;

///////////////////// with std::allocator and Regular types /////////////////////
// sequence containers
CONCEPT_ASSERT(OrderedContainer<std::deque<bool>>());
CONCEPT_ASSERT(!OrderedContainer<std::forward_list<float>>()); // TODO: decide if forward_list is an exemption... if so, what defines an "exemption"?
CONCEPT_ASSERT(OrderedContainer<std::list<double>>());
CONCEPT_ASSERT(OrderedContainer<std::vector<int>>());

CONCEPT_ASSERT(ranges::SequenceContainer<std::vector<int>>());

// associative containers
CONCEPT_ASSERT(OrderedContainer<std::map<int, std::string>>());
CONCEPT_ASSERT(OrderedContainer<std::set<std::vector<int>>>());
CONCEPT_ASSERT(ranges::Container<std::unordered_map<std::string, int>>());
CONCEPT_ASSERT(!OrderedContainer<std::unordered_map<std::string, int>>());
CONCEPT_ASSERT(ranges::Container<std::unordered_set<std::string>>());
CONCEPT_ASSERT(!OrderedContainer<std::unordered_set<std::string>>());

// container adaptors
CONCEPT_ASSERT(!OrderedContainer<std::priority_queue<short>>());
CONCEPT_ASSERT(!OrderedContainer<std::queue<long long>>());
CONCEPT_ASSERT(!OrderedContainer<std::stack<long double>>());

// Bjarne's "almost containers"
CONCEPT_ASSERT(OrderedContainer<std::array<int, 10>>()); // I think this one is okay to pass
CONCEPT_ASSERT(!Container<int[]>());
CONCEPT_ASSERT(!Container<std::bitset<8>>());
CONCEPT_ASSERT(!Container<std::vector<bool>>()); // I prefer Scott's classification to Bjarne's ;)
CONCEPT_ASSERT(!Container<std::pair<int, float>>());
CONCEPT_ASSERT(!Container<std::tuple<>>());
CONCEPT_ASSERT(!Container<std::tuple<int>>());
CONCEPT_ASSERT(!Container<std::tuple<int, double>>());
CONCEPT_ASSERT(!Container<std::tuple<int, double, std::vector<int>>>());
CONCEPT_ASSERT(OrderedContainer<std::string>()); // I think this is one is okay to pass
CONCEPT_ASSERT(!Container<std::valarray<int>>());

///////////////////// with std::allocator and non-Movable types /////////////////////

///////////////////// with custom allocator /////////////////////

void foo(OrderedContainer);

int main()
{
   return ::test_result();
}