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
#include <stl2/algorithm.hpp>
#include <stl2/vector.hpp>
#include <stl2/view/repeat_n.hpp>
#include <iostream>
#include "../cmcstl2/test/simple_test.hpp"

namespace ranges = std::experimental::ranges;

template <class> class show_type;

struct A {
	using value_type = int;

	int* allocate(int);
	void deallocate(int*, int);

	friend bool operator==(A, A) { return true; }
	friend bool operator!=(A, A) { return false; }
};
static_assert(ranges::models::ProtoAllocator<A>);
static_assert(ranges::models::Same<void*,
	ranges::proto_allocator_pointer_t<A>>);
static_assert(ranges::models::CopyConstructible<A>);
static_assert(ranges::models::EqualityComparable<A>);
static_assert(ranges::models::Same<int*,
	ranges::allocator_pointer_t<A>>);
static_assert(ranges::models::RebindablePointer<int*, void>);
static_assert(ranges::models::RebindablePointer<void*, int>);
static_assert(ranges::models::Same<void*,
	ranges::allocator_void_pointer_t<A>>);
static_assert(ranges::models::Allocator<A, int>);

static_assert(ranges::models::NullablePointer<void*>);
static_assert(ranges::models::Pointer<void*>);
static_assert(ranges::models::PointerTo<void*, void>);

static_assert(ranges::models::NullablePointer<int*>);
static_assert(ranges::models::Pointer<int*>);
static_assert(ranges::models::PointerTo<int*, int>);
static_assert(ranges::models::DereferenceablePointer<int*>);
static_assert(ranges::models::ProtoAllocator<std::allocator<int>>);
static_assert(ranges::models::Same<void*,
	ranges::proto_allocator_pointer_t<std::allocator<int>>>);
static_assert(ranges::models::CopyConstructible<std::allocator<int>>);
static_assert(ranges::models::EqualityComparable<std::allocator<int>>);
static_assert(ranges::models::Same<int*,
	ranges::allocator_pointer_t<std::allocator<int>>>);
static_assert(ranges::models::Same<void*,
	ranges::allocator_void_pointer_t<std::allocator<int>>>);
static_assert(ranges::models::Allocator<std::allocator<int>, int>);

namespace incomplete {
	struct foo;
	static_assert(ranges::models::Same<foo*, ranges::common_type_t<foo*,foo*>>);
	static_assert(ranges::models::Common<foo*,foo*>);
	static_assert(ranges::models::EqualityComparable<foo*>);
	static_assert(ranges::models::NullablePointer<foo*>);
	static_assert(ranges::models::ProtoAllocator<std::allocator<foo>>);
	static_assert(ranges::models::ProtoAllocator<std::allocator<foo>, foo>);
	ranges::vector<foo> vec;
	static_assert(ranges::models::Semiregular<foo*>);
	static_assert(ranges::models::Same<std::ptrdiff_t, ranges::difference_type_t<foo*>>);

	struct foo {};
	static_assert(ranges::models::Same<foo*&, decltype(++ranges::declval<foo*&>())>);
	static_assert(ranges::models::Same<decltype(++ranges::declval<foo*&>()), foo*&>);
	static_assert(ranges::models::Same<foo*, decltype(ranges::declval<foo*&>()++)>);
	static_assert(ranges::models::Same<decltype(ranges::declval<foo*&>()++), foo*>);
	static_assert(ranges::models::Allocator<std::allocator<foo>, foo>);
	static_assert(ranges::models::ContiguousIterator<foo*>);

	void test() {
		vec.emplace_back();
	}
}

int main() {
	{
		ranges::vector<int> vec;
		vec.push_back(42);
		for (auto i = 1; i < 32; ++i) {
			vec.push_back(vec.back());
		}
		CHECK(ranges::equal(vec, ranges::ext::repeat_n_view<int>{42, 32}));
		for (auto i = 0; i < 16; ++i) {
			vec.pop_back();
		}
		CHECK(ranges::equal(vec, ranges::ext::repeat_n_view<int>{42, 16}));
	}

	incomplete::test();

	return ::test_result();
}
