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
#ifndef STL2_DETAIL_CONCEPTS_ALLOCATOR_HPP
#define STL2_DETAIL_CONCEPTS_ALLOCATOR_HPP

#include <stl2/detail/fwd.hpp>
#include <stl2/detail/concepts/pointer.hpp>
#include <stl2/detail/iterator/increment.hpp>
#include <memory>

// Note: basically none of this will work without constrained allocator_traits.
STL2_OPEN_NAMESPACE {
  namespace __allocator {
    template <class C>
    concept bool MemberVoidPointer =
      requires { typename C::void_pointer; };

    template <class C>
    concept bool MemberPointer =
      requires { typename C::pointer; };

    template <class>
    struct proto_pointer {
      using type = void*;
    };

    template <MemberPointer P>
      requires !MemberVoidPointer<P> &&
        RebindablePointer<typename P::pointer, void>()
    struct proto_pointer<P> {
      using type = rebind_pointer_t<typename P::pointer, void>;
    };

    MemberVoidPointer{P}
    struct proto_pointer<P> {
      using type = typename P::void_pointer;
    };
  }

  template <_Is<is_class> P>
    requires _Valid<meta::_t, __allocator::proto_pointer<P>> &&
      PointerTo<meta::_t<__allocator::proto_pointer<P>>, void>()
  using proto_allocator_pointer_t =
    meta::_t<__allocator::proto_pointer<P>>;

  template <class P>
  concept bool ProtoAllocator() {
    return requires {
      typename proto_allocator_pointer_t<P>;
    };
  }

  namespace __allocator {
    template <class A>
    struct pointer {
      using type = typename A::value_type*;
    };
    template <class A>
      requires requires { typename A::pointer; }
    struct pointer<A> {
      using type = typename A::pointer;
    };
  }
  template <class A>
    requires _Valid<meta::_t, __allocator::pointer<A>> &&
      PointerTo<meta::_t<__allocator::pointer<A>>, typename A::value_type>()
  using allocator_pointer_t =
    meta::_t<__allocator::pointer<A>>;

  namespace __allocator {
    template <class>
    struct void_pointer {};
    MemberVoidPointer{A}
    struct void_pointer<A> {
      using type = typename A::void_pointer;
    };
    template <class A>
      requires !MemberVoidPointer<A> && requires {
        typename allocator_pointer_t<A>;
        requires RebindablePointer<allocator_pointer_t<A>, void>();
      }
    struct void_pointer<A> {
      using type = rebind_pointer_t<allocator_pointer_t<A>, void>;
    };
  }
  template <class A>
    requires _Valid<meta::_t, __allocator::void_pointer<A>> &&
      PointerTo<meta::_t<__allocator::void_pointer<A>>, void>()
  using allocator_void_pointer_t =
    meta::_t<__allocator::void_pointer<A>>;

  namespace __allocator {
    template <class>
    struct difference_type {};
    detail::MemberDifferenceType{A}
    struct difference_type<A> {
      using type = typename A::difference_type;
    };
    template <class A>
      requires !detail::MemberDifferenceType<A> &&
        _Valid<allocator_pointer_t, A>
    struct difference_type<A> {
      using type = difference_type_t<allocator_pointer_t<A>>;
    };
  }
  template <class A>
    requires _Valid<meta::_t, __allocator::difference_type<A>> &&
      SignedIntegral<meta::_t<__allocator::difference_type<A>>>()
  using allocator_difference_t =
    meta::_t<__allocator::difference_type<A>>;

  namespace __allocator {
    template <class A>
    concept bool MemberSizeType = requires { typename A::size_type; };

    template <class>
    struct size_type {};
    MemberSizeType{A}
    struct size_type<A> {
      using type = typename A::size_type;
    };
    template <class A>
      requires !MemberSizeType<A> && _Valid<allocator_difference_t, A>
    struct size_type<A> : make_unsigned<allocator_difference_t<A>> {};
  }
  template <class A>
    requires _Valid<meta::_t, __allocator::size_type<A>> &&
      Integral<meta::_t<__allocator::size_type<A>>>()
  using allocator_size_t =
    meta::_t<__allocator::size_type<A>>;

  template <class A, class T>
  concept bool Allocator() {
    return _Is<T, is_object> &&
      CopyConstructible<A>() &&
      EqualityComparable<A>() &&
      requires {
        typename A::value_type;
        requires Same<T, typename A::value_type>();
        typename allocator_pointer_t<A>;
        typename allocator_void_pointer_t<A>;
        typename allocator_size_t<A>;
        // ...
      } && requires (A a, const allocator_pointer_t<A> p, const allocator_size_t<A> n) {
        // not required to be equality preserving
        { a.allocate(n) } -> allocator_pointer_t<A>;
        (void)a.deallocate(p, n); /* noexcept */
      };
    // Axiom: move and copy construction do not throw.
  }

  namespace models {
    template <class, class>
    constexpr bool Allocator = false;
    __stl2::Allocator{A, T}
    constexpr bool Allocator<A, T> = true;
  }

  namespace __allocator {
    template <class, class>
    struct rebind {};
    template <class A, class T>
      requires requires { typename A::template rebind<T>::other; }
    struct rebind<A, T> {
      using type = typename A::template rebind<T>::other;
    };
    template <template <class...> class A, class T, class...Args, class U>
      requires !(requires { typename A<T, Args...>::template rebind<U>::other; }) &&
        _Valid<A, U, Args...>
    struct rebind<A<T, Args...>, U> {
      using type = A<U, Args...>;
    };
  }

  template <ProtoAllocator PA, _Is<is_object> T>
  requires
    _Valid<meta::_t, __allocator::rebind<PA, T>>
      && Same<proto_allocator_pointer_t<PA>,
           allocator_void_pointer_t<meta::_t<__allocator::rebind<PA, T>>>>()
  using rebind_allocator_t =
    meta::_t<__allocator::rebind<PA, T>>;

  template <class PA, class T>
  concept bool ProtoAllocator() {
    return ProtoAllocator<PA>() && _Is<T, is_object> &&
      requires { typename rebind_allocator_t<PA, T>; };
  }

  namespace models {
    template <class PA, class T = void>
    constexpr bool ProtoAllocator = false;
    __stl2::ProtoAllocator{PA}
    constexpr bool ProtoAllocator<PA, void> = true;
    __stl2::ProtoAllocator{PA, T}
    constexpr bool ProtoAllocator<PA, T> = true;
  }

  template <class A, class T>
  concept bool AllocatorDestructible() {
    return requires (A& a, T* p) {
      { (void)std::allocator_traits<A>::destroy(a, p) } /* noexcept */;
    };
  }

  template <class A, class T, class...Args>
  concept bool AllocatorConstructible() {
    return AllocatorDestructible<A, T>() &&
      requires (A& a, T* p, Args&&...args) {
        (void)std::allocator_traits<A>::construct(a, p, (Args&&)args...);
      };
  }

  template <class A, class T>
  concept bool AllocatorDefaultConstructible() {
    return AllocatorConstructible<A, T>();
  }

  template <class A, class T>
  concept bool AllocatorMoveConstructible() {
    return AllocatorConstructible<A, T, T>();
  }

  template <class A, class T>
  concept bool AllocatorCopyConstructible() {
    return AllocatorMoveConstructible<A, T>() &&
      AllocatorConstructible<A, T, const T&>();
  }
} STL2_CLOSE_NAMESPACE

#endif
