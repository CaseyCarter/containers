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
#ifndef STL2_DETAIL_CONCEPTS_POINTER_HPP
#define STL2_DETAIL_CONCEPTS_POINTER_HPP

#include <stl2/detail/fwd.hpp>
#include <stl2/detail/meta.hpp>
#include <stl2/detail/concepts/compare.hpp>
#include <stl2/detail/concepts/core.hpp>
#include <stl2/detail/concepts/object.hpp>
#include <stl2/detail/iterator/concepts.hpp>

STL2_OPEN_NAMESPACE {
	template <class P>
	concept bool NullablePointer() {
		return Regular<P>() &&
			Constructible<P, const std::nullptr_t&>() &&
			ConvertibleTo<std::nullptr_t, P>() &&
			EqualityComparable<P, std::nullptr_t>() &&
			Assignable<P&, const std::nullptr_t&>() &&
			requires(P& p) {
				static_cast<bool>(p);
			};
		// Axiom: P{} is equality preserving
		// Axiom: P{nullptr} is equality preserving
		// Axiom: P{} == P{nullptr}
		// Axiom: P{} does not denote an object
		// Axiom: (p = nullptr, p) == P{nullptr}
		// Axiom: two non-P{} NullablePointers are equal iff they denote the same object
	}

	namespace models {
		template <class>
		constexpr bool NullablePointer = false;
		__stl2::NullablePointer{P}
		constexpr bool NullablePointer<P> = true;
	}

	template <class>
	struct element_type {};

	template <class T>
	struct element_type<T*> {
		using type = T;
	};

	template <class P>
		requires requires { typename P::element_type; }
	struct element_type<P> {
		using type = typename P::element_type;
	};

	template <class P>
		requires _Valid<meta::_t, element_type<P>> &&
			_IsNot<meta::_t<element_type<P>>, is_reference>
	using element_type_t = meta::_t<element_type<P>>;

	template <class P>
	concept bool Pointer() {
		return NullablePointer<P>() && requires {
			typename element_type_t<P>;
		};
	}

	namespace models {
		template <class>
		constexpr bool Pointer = false;
		__stl2::Pointer{P}
		constexpr bool Pointer<P> = true;
	}

	template <class P, class T>
	concept bool PointerTo() {
		return Pointer<P>() && Same<T, element_type_t<P>>();
	}

	namespace models {
		template <class, class>
		constexpr bool PointerTo = false;
		__stl2::PointerTo{P, T}
		constexpr bool PointerTo<P, T> = true;
	}

	template <class P>
	concept bool DereferenceablePointer() {
		return Pointer<P>() && RandomAccessIterator<P>() &&
			Same<reference_t<P>, element_type_t<P>&>();
	}

	namespace models {
		template <class>
		constexpr bool DereferenceablePointer = false;
		__stl2::DereferenceablePointer{P}
		constexpr bool DereferenceablePointer<P> = true;
	}

	template <Pointer, _IsNot<is_reference> >
	struct rebind_pointer {};
	template <class T, class U>
	struct rebind_pointer<T*, U> {
		using type = U*;
	};
	template <Pointer P, _IsNot<is_reference> U>
		requires requires { typename P::template rebind<U>; }
	struct rebind_pointer<P, U> {
		using type = typename P::template rebind<U>;
	};
	template <template <class...> class P, class T, class...Args, class U>
		requires !(requires { typename P<T, Args...>::template rebind<U>; }) &&
			_Valid<P, U, Args...>
	struct rebind_pointer<P<T, Args...>, U> {
		using type = P<U, Args...>;
	};

	template <Pointer P, _IsNot<is_reference> T>
		requires _Valid<meta::_t, rebind_pointer<P, T>> &&
			PointerTo<meta::_t<rebind_pointer<P, T>>, T>()
	using rebind_pointer_t = meta::_t<rebind_pointer<P, T>>;

	template <class P, class T>
	concept bool RebindablePointer() {
		return requires {
			typename rebind_pointer_t<P, T>;
			requires Same<P, rebind_pointer_t<rebind_pointer_t<P, T>, element_type_t<P>>>(); // Paranoia
		};
	}

	namespace models {
		template <class, class>
		constexpr bool RebindablePointer = false;
		__stl2::RebindablePointer{P, T}
		constexpr bool RebindablePointer<P, T> = true;
	}

	// FIXME: I hate this entire design of pointer_to.
	namespace detail {
		template <class T, class U>
			requires ConvertibleTo<U*, T*>()
		T* pointer_to(meta::id<T*>, U& u) noexcept {
			return std::addressof(u);
		}
	}

	template <Pointer P, class U>
		requires requires (U& u) {
			{ detail::pointer_to(meta::id<P>{}, u) } -> P;
		}
	P pointer_to(U& u)
	STL2_NOEXCEPT_RETURN(
		detail::pointer_to(meta::id<P>{}, u)
	)
} STL2_CLOSE_NAMESPACE

#endif
