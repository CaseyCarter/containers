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
#ifndef STL2_FORWARD_LIST_HPP
#define STL2_FORWARD_LIST_HPP

#ifndef STL2_FORWARD_USELESS_TRASH
#define STL2_FORWARD_USELESS_TRASH 1
#endif // STL2_FORWARD_USELESS_TRASH

#include <stl2/memory.hpp>
#include <stl2/type_traits.hpp>
#include <stl2/utility.hpp>
#include <stl2/detail/ebo_box.hpp>
#include <stl2/detail/fwd.hpp>
#include <stl2/detail/algorithm/equal.hpp>
#include <stl2/detail/algorithm/lexicographical_compare.hpp>
#include <stl2/detail/concepts/allocator.hpp>
#include <stl2/detail/concepts/core.hpp>
#include <stl2/detail/concepts/pointer.hpp>
#include <stl2/detail/iterator/basic_iterator.hpp>
#include <stl2/detail/iterator/concepts.hpp>
#include <stl2/detail/iterator/move_iterator.hpp>

STL2_OPEN_NAMESPACE {
	namespace __fl {
		template <class T, PointerTo<void> VoidPointer>
		struct node {
			using pointer = rebind_pointer_t<VoidPointer, node>;

			node() = default;
			node(const node&) = delete;
			node& operator=(const node&) & = delete;

			T& get() & noexcept { return reinterpret_cast<T&>(storage_); }
			const T& get() const& noexcept { return reinterpret_cast<const T&>(storage_); }

			pointer next() const noexcept {
				return next_;
			}

			void push(pointer p) noexcept {
				STL2_EXPECT(p);
				p->next_ = std::move(next_);
				next_ = std::move(p);
			}

			pointer pop() noexcept {
				STL2_EXPECT(next_);
				return __stl2::exchange(next_, next_->next_);
			}

			pointer next_;
			std::aligned_storage_t<sizeof(T), alignof(T)> storage_;
		};

		template <class T, PointerTo<void> VoidPointer>
		struct cursor {
			using value_type = std::remove_cv_t<T>;
			using difference_type = std::ptrdiff_t;
			using node_t = node<value_type, VoidPointer>;
			using node_pointer = rebind_pointer_t<VoidPointer, node_t>;

			cursor() = default;
			constexpr cursor(default_sentinel) noexcept
			: pos_{nullptr} {}
			template <class U>
			requires
				std::is_const<T>::value &&
				Same<U, std::remove_const_t<T>>()
			constexpr cursor(const cursor<U, VoidPointer>& that) noexcept
			: pos_{that.pos_} {}

			T& read() const noexcept {
				STL2_EXPECT(pos_);
				return reinterpret_cast<node_t*>(pos_)->get();
			}
			void next() noexcept {
				STL2_EXPECT(pos_);
				pos_ = std::addressof((*pos_)->next_);
			}
			constexpr bool equal(const cursor& that) const noexcept {
				return pos_ == that.pos_;
			}
			constexpr bool done() const noexcept {
				return pos_ == nullptr;
			}
		private:
			explicit constexpr cursor(node_pointer* pos) noexcept
			: pos_{std::move(pos)} {}

			node_pointer* pos_;
		};

		template <class T, PointerTo<void> VoidPointer>
		class base {
		protected:
			rebind_pointer_t<VoidPointer, node<T, VoidPointer>> head_ = nullptr;

			using cursor = __fl::cursor<T, VoidPointer>;
			using const_cursor = __fl::cursor<const T, VoidPointer>;

		public:
			using value_type = T;
			using iterator = __stl2::basic_iterator<cursor>;
			using const_iterator = __stl2::basic_iterator<const_cursor>;

			base() = default;
			base(base&& that) noexcept
			: head_(__stl2::exchange(that.head_, {})) {}

			base& operator=(base&&) & = delete;

			iterator before_begin() noexcept {
				return cursor{std::addressof(head_)};
			}
			const_iterator before_begin() const noexcept {
				return const_cursor{std::addressof(head_)};
			}
			const_iterator cbefore_begin() const noexcept {
				return before_begin();
			}

			iterator begin() noexcept {
				return cursor{head_ ? std::addressof(head_->next_): nullptr};
			}
			const_iterator begin() const noexcept {
				return const_cursor{head_ ? std::addressof(head_->next_) : nullptr};
			}
			const_iterator cbegin() const noexcept {
				return begin();
			}

			default_sentinel end() const noexcept {
				return {};
			}
			default_sentinel cend() const noexcept {
				return {};
			}

			T& front() noexcept {
				STL2_EXPECT(head_);
				return head_->get();
			}
			const T& front() const noexcept {
				STL2_EXPECT(head_);
				return head_->get();
			}
		};
	}

	template <class T, ProtoAllocator A = std::allocator<T>>
	requires
		ProtoAllocator<A, __fl::node<T, proto_allocator_pointer_t<A>>>()
	class forward_list
		: public __fl::base<T, proto_allocator_pointer_t<A>>
		, detail::ebo_box<A>
	{
		using base_t = __fl::base<T, proto_allocator_pointer_t<A>>;
		using typename base_t::cursor;
		using typename base_t::const_cursor;
		using base_t::head_;

		using alloc_base_t = detail::ebo_box<A>;

		using node_t = __fl::node<T, proto_allocator_pointer_t<A>>;
		using node_allocator_type = rebind_allocator_t<A, node_t>;
		using node_pointer = allocator_pointer_t<node_allocator_type>;
		using traits = std::allocator_traits<node_allocator_type>;

	public:
		using typename base_t::value_type;
		using allocator_type = A;
#if STL2_FORWARD_USELESS_TRASH
		using pointer = allocator_pointer_t<A>;
		using const_pointer = rebind_pointer_t<pointer, const T>;
		using reference = T&;
		using const_reference = const T&;
		using size_type = allocator_difference_t<A>;
		using difference_type = allocator_difference_t<A>;
#endif // STL2_FORWARD_USELESS_TRASH
		using typename base_t::iterator;
		using typename base_t::const_iterator;

		~forward_list()
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorDestructible<node_allocator_type, T>()
		{ clear(); }

		forward_list()
		requires
			DefaultConstructible<A>() &&
			AllocatorDestructible<node_allocator_type, T>() = default;

		constexpr explicit forward_list(allocator_type a) noexcept
		requires AllocatorDestructible<node_allocator_type, T>()
		: alloc_base_t(std::move(a)) {}

		explicit forward_list(size_type n, allocator_type a)
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorDefaultConstructible<node_allocator_type, T>()
		: forward_list{std::move(a)}
		{
			while (n-- > 0) {
				emplace_front();
			}
		}
		explicit forward_list(size_type n)
		requires
			DefaultConstructible<A>() &&
			Allocator<node_allocator_type, node_t>() &&
			AllocatorDefaultConstructible<node_allocator_type, T>()
		: forward_list{n, allocator_type{}}
		{}

#if STL2_FORWARD_USELESS_TRASH
		// Use forward_list{view::repeat_n(value, n)}
		forward_list(size_type n, const T& value, allocator_type a)
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorCopyConstructible<node_allocator_type, T>()
		: forward_list{std::move(a)}
		{
			while (n-- > 0) {
				emplace_front(value);
			}
		}
		forward_list(size_type n, const T& value)
		requires
			DefaultConstructible<A>() &&
			Allocator<node_allocator_type, node_t>() &&
			AllocatorCopyConstructible<node_allocator_type, T>()
		: forward_list{n, value, allocator_type{}}
		{}
#endif // STL2_FORWARD_USELESS_TRASH

		template <InputIterator I, Sentinel<I> S>
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorConstructible<node_allocator_type, T, reference_t<I>>()
		forward_list(I first, S last, allocator_type a)
		: forward_list{std::move(a)}
		{
			insert_after(before_begin(), std::move(first), std::move(last));
		}
		template <InputIterator I, Sentinel<I> S>
		requires
			DefaultConstructible<A>() &&
			Allocator<node_allocator_type, node_t>() &&
			AllocatorConstructible<node_allocator_type, T, reference_t<I>>()
		forward_list(I first, S last)
		: forward_list{}
		{
			insert_after(before_begin(), std::move(first), std::move(last));
		}
		template <InputRange Rng>
		requires
			AllocatorConstructible<node_allocator_type, T, reference_t<iterator_t<Rng>>>()
		forward_list(Rng&& rng, allocator_type a)
		: forward_list{__stl2::begin(rng), __stl2::end(rng), std::move(a)}
		{}
		template <InputRange Rng>
		requires
			DefaultConstructible<allocator_type>() &&
			Allocator<node_allocator_type, node_t>() &&
			AllocatorConstructible<node_allocator_type, T, reference_t<iterator_t<Rng>>>()
		forward_list(Rng&& rng)
		: forward_list{__stl2::begin(rng), __stl2::end(rng)}
		{}

		forward_list(const forward_list& that, allocator_type a)
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorCopyConstructible<node_allocator_type, T>()
		: forward_list{std::move(a)}
		{
			insert_after(before_begin(), that);
		}
		forward_list(const forward_list& that)
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorCopyConstructible<node_allocator_type, T>()
		: forward_list{that,
			std::allocator_traits<A>::select_on_container_copy_construction(
				that.alloc())}
		{}

		forward_list(forward_list&& that) noexcept
		: base_t{std::move(that)}
		, alloc_base_t{std::move(that.alloc())} {}
		forward_list(forward_list&& that, allocator_type a) noexcept
		: forward_list{std::move(a)}
		{
			if (traits::is_always_equal::value || that.alloc() == a) {
				head_ = __stl2::exchange(that.head, {});
			} else {
				insert_after(before_begin(),
					__stl2::make_move_iterator(that.begin()),
					__stl2::make_move_sentinel(that.end()));
			}
		}

		forward_list& operator=(forward_list&& that) &
		noexcept(traits::is_always_equal::value ||
			traits::propagate_on_container_move_assignment::value)
		requires
			Allocator<node_allocator_type, node_t>() &&
			Movable<T>() &&
			AllocatorMoveConstructible<node_allocator_type, T>()
		{
			if (std::addressof(that) != this) {
				if (traits::is_always_equal::value || traits::propagate_on_container_move_assignment::value ||
					alloc() == that.alloc()) {
					if (traits::is_always_equal::value || traits::propagate_on_container_move_assignment::value) {
						clear();
						if (traits::propagate_on_container_move_assignment::value) {
							alloc() = std::move(that.alloc());
						}
					}
					head_ = __stl2::exchange(that.head_, {});
				} else {
					assign(
						__stl2::make_move_iterator(that.begin()),
						__stl2::make_move_sentinel(that.end()));
				}
			}
			return *this;
		}
		forward_list& operator=(const forward_list& that) &
		requires
			Allocator<node_allocator_type, node_t>() &&
			Copyable<T>() &&
			AllocatorCopyConstructible<node_allocator_type, T>()
		{
			if (std::addressof(that) != this) {
				if (!traits::is_always_equal::value &&
					traits::propagate_on_container_copy_assignment::value &&
					!(alloc() == that.alloc()))
				{
					clear();
					alloc() = that.alloc();
				}
				assign(that.begin(), that.end());
			}
			return *this;
		}
		template <InputRange Rng>
		requires
			Allocator<node_allocator_type, node_t>() &&
			Assignable<T&, reference_t<iterator_t<Rng>>>() &&
			AllocatorConstructible<node_allocator_type, T, reference_t<iterator_t<Rng>>>()
		forward_list& operator=(Rng&& rng) &
		{
			assign(std::forward<Rng>(rng));
			return *this;
		}

		template <InputIterator I, Sentinel<I> S>
		requires
			Allocator<node_allocator_type, node_t>() &&
			Assignable<T&, reference_t<I>>() &&
			AllocatorConstructible<node_allocator_type, T, reference_t<I>>()
		void assign(I first, S last) {
			for (auto p = before_begin();; ++first) {
				if (first == last) {
					erase_after(p, end());
					break;
				}
				if (!*p.pos_) {
					insert_after(p, std::move(first), std::move(last));
					break;
				}
				*++p = *first;
			}
		}
		template <InputRange Rng>
		requires
			Allocator<node_allocator_type, node_t>() &&
			Assignable<T&, reference_t<iterator_t<Rng>>>() &&
			AllocatorConstructible<node_allocator_type, T, reference_t<iterator_t<Rng>>>()
		void assign(Rng&& rng) {
			assign(__stl2::begin(rng), __stl2::end(rng));
		}

		using base_t::before_begin;
		using base_t::begin;
		using base_t::end;
		using base_t::cbefore_begin;
		using base_t::cbegin;
		using base_t::cend;

		void swap(forward_list& that)
		noexcept(traits::is_always_equal::value || traits::propagate_on_container_swap::value)
		{
			if (traits::propagate_on_container_swap::value) {
				__stl2::swap(alloc(), that.alloc());
			} else if (!traits::is_always_equal::value) {
				STL2_EXPECT(alloc() == that.alloc());
			}
			__stl2::swap(head_, that.head_);
		}
		friend void swap(forward_list& lhs, forward_list& rhs)
		noexcept(noexcept(lhs.swap(rhs)))
		{
			lhs.swap(rhs);
		}

		allocator_type get_allocator() const noexcept {
			return alloc();
		}

		template <class...Args>
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorConstructible<node_allocator_type, T, Args...>()
		iterator emplace_after(const_iterator where, Args&&...args) {
			auto a = node_allocator_type{alloc()};
			Same<node_pointer> new_node = traits::allocate(a, 1);
			try {
				traits::construct(a, std::addressof(new_node->get()),
					__stl2::forward<Args>(args)...);
			} catch(...) {
				traits::deallocate(a, new_node, 1);
				throw;
			}
			new_node->next_ = *where.pos_;
			*where.pos_ = new_node;
			return cursor{std::addressof(new_node->next_)};
		}
		template <class...Args>
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorConstructible<node_allocator_type, T, Args...>()
		T& emplace_front(Args&&...args) {
			return *emplace_after(before_begin(), std::forward<Args>(args)...);
		}

		void push_front(const T& t)
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorCopyConstructible<node_allocator_type, T>()
		{
			emplace_front(t);
		}
		void push_front(T&& t)
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorMoveConstructible<node_allocator_type, T>()
		{
			emplace_front(std::move(t));
		}

		void pop_front() noexcept
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorDestructible<node_allocator_type, T>()
		{
			STL2_EXPECT(head_);
			auto a = node_allocator_type{alloc()};
			node_pointer tmp = head_;
			head_ = head_->next_;
			traits::destroy(a, std::addressof(tmp->get()));
			traits::deallocate(a, tmp, 1);
		}

		void clear() noexcept
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorDestructible<node_allocator_type, T>()
		{
			erase_after(before_begin(), end());
		}

		template<InputIterator I, Sentinel<I> S>
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorConstructible<node_allocator_type, T, reference_t<I>>()
		iterator insert_after(const_iterator where, I first, S const last)
		{
			iterator pos{cursor{where.pos_}};
			for (; first != last; ++first) {
				pos = emplace_after(pos, *first);
			}
			return pos;
		}
		template<InputRange Rng>
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorConstructible<node_allocator_type, T, reference_t<iterator_t<Rng>>>()
		iterator insert_after(const_iterator where, Rng&& rng)
		{
			return insert_after(std::move(where), __stl2::begin(rng), __stl2::end(rng));
		}

		void erase_after(const_iterator first, default_sentinel) noexcept
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorDestructible<node_allocator_type, T>()
		{
			erase_after_(std::move(first), node_pointer{});
		}

		void erase_after(const_iterator first, const_iterator last) noexcept
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorDestructible<node_allocator_type, T>()
		{
			erase_after_(std::move(first), *last.pos_);
		}

	private:
		A& alloc() { return alloc_base_t::get(); }
		const A& alloc() const { return alloc_base_t::get(); }

		void erase_after_(const_iterator first, node_pointer last) noexcept
		requires
			Allocator<node_allocator_type, node_t>() &&
			AllocatorDestructible<node_allocator_type, T>()
		{
			auto a = node_allocator_type{alloc()};
			auto ptr = __stl2::exchange(*first.pos_, last);
			while (ptr != last) {
				auto tmp = __stl2::exchange(ptr, ptr->next_);
				traits::destroy(a, std::addressof(tmp->get()));
				traits::deallocate(a, tmp, 1);
			}
		}
	};

	template <class T, class TA, class U, class UA>
	requires EqualityComparable<T, U>()
	bool operator==(const forward_list<T, TA>& t, const forward_list<U, UA>& u) {
		return __stl2::equal(t, u);
	}
	template <class T, class TA, class U, class UA>
	requires EqualityComparable<T, U>()
	inline bool operator!=(const forward_list<T, TA>& t, const forward_list<U, UA>& u) {
		return !(t == u);
	}
	template <class T, class TA, class U, class UA>
	requires StrictTotallyOrdered<T, U>()
	bool operator<(const forward_list<T, TA>& t, const forward_list<U, UA>& u) {
		return __stl2::lexicographical_compare(t, u);
	}
	template <class T, class TA, class U, class UA>
	requires StrictTotallyOrdered<T, U>()
	bool operator>(const forward_list<T, TA>& t, const forward_list<U, UA>& u) {
		return (u < t);
	}
	template <class T, class TA, class U, class UA>
	requires StrictTotallyOrdered<T, U>()
	bool operator<=(const forward_list<T, TA>& t, const forward_list<U, UA>& u) {
		return !(u < t);
	}
	template <class T, class TA, class U, class UA>
	requires StrictTotallyOrdered<T, U>()
	bool operator>=(const forward_list<T, TA>& t, const forward_list<U, UA>& u) {
		return !(t < u);
	}
} STL2_CLOSE_NAMESPACE

#endif
