// cmcstl2 - A concept-enabled C++ standard library
//
//  Copyright Casey Carter 2015
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
#include <stl2/forward_list.hpp>
#include <stl2/algorithm.hpp>
#include <stl2/view/iota.hpp>
#include <stl2/view/repeat_n.hpp>
#include <stl2/view/take_exactly.hpp>
#include <iostream>
#include "../cmcstl2/test/simple_test.hpp"

namespace ranges = std::experimental::ranges;

void dump(const ranges::ext::StreamInsertable& t) {
	std::cout << t;
}

void dump(ranges::Range&& r) {
	std::cout << '{';
	auto pos = ranges::begin(r);
	auto end = ranges::end(r);
	if (pos != end) {
		dump(*pos);
		while (++pos != end) {
			std::cout << ", ";
			dump(*pos);
		}
	}
	std::cout << '}';
}

namespace incomplete {
	ranges::forward_list<struct foo> list;
	struct foo {};

	void test() {
		list.emplace_front();
	}
}

struct S {};

int main() {
	{
		ranges::forward_list<int>{};
		ranges::forward_list<S>{};
	}

	{
		using L = ranges::forward_list<int>;
		using I = decltype(ranges::declval<L&>().begin());
		using S = decltype(ranges::declval<L&>().end());
		static_assert(ranges::models::ForwardIterator<I>);
		static_assert(ranges::models::Sentinel<S, I>);
		static_assert(ranges::models::ForwardRange<L>);

		L list{};
		for (auto i = 4; i-- != 0;) {
			list.push_front(i);
		}
		::check_equal(list, {0, 1, 2, 3});
	}

	{
		auto n = 0;
		ranges::forward_list<ranges::forward_list<int>> ll{};
		for (auto i = 0; i < 4; ++i) {
			ll.emplace_front();
			auto& l = ll.front();
			for (auto e = n + 4; n < e; ++n) {
				l.push_front(n);
			}
		}
		dump(ll);
		std::cout << '\n';
	}

	{
		ranges::forward_list<int> list{ranges::repeat_n_view<int>{42, 4}};
		CHECK(ranges::distance(list) == 4);
		CHECK(ranges::equal(list, ranges::repeat_n_view<int>{42, 4}));
	}
	{
		ranges::forward_list<S> list{ranges::repeat_n_view<S>{{}, 8}};
		CHECK(ranges::distance(list) == 8);
	}
	{
		int some_ints[] = {4, 5, 6, 7, 3, 2, 1, 0};
		ranges::forward_list<int> list{some_ints};
		CHECK(ranges::equal(list, some_ints));
		ranges::sort(list);
		CHECK(ranges::is_sorted(list));
	}

	{
		ranges::forward_list<int> list{ranges::take_exactly_view<ranges::iota_view<int>>{{}, 8}};
		CHECK(ranges::distance(list) == 8);
		CHECK(ranges::is_sorted(list));
		auto l2 = list;
		CHECK(ranges::equal(list, l2));
	}
	{
		ranges::forward_list<S> list{ranges::repeat_n_view<S>{{}, 8}};
		CHECK(ranges::distance(list) == 8);
		auto l2 = list;
		CHECK(ranges::distance(l2) == 8);
	}

	incomplete::test();

	return ::test_result();
}
