#pragma once // basic introspection, traits and meta data on types and functions
#include "val_base.h"
#include <type_traits>

template<class T> using VOID = void;

template<class A, class B> using same_t = std::is_same<A, B>;
template<class A, class B> constexpr bool same() { return same_t<A,B>::value; } // A == B
static_assert(same<int, int>(), "same");

template<class T> using remove_cv = typename std::remove_cv<T>::type;

template<class A, class B> constexpr bool convertible() { return std::is_convertible<A,B>::value; } // B(a) valid
template<class T> constexpr bool zero_size() { return std::is_empty<T>::value; } // sizeof(T) == 0
using std::declval; // oposite of decltype

template<class... Ts> using common = typename std::common_type<Ts...>::type;
template<class A, class B> using common2_t = std::common_type<A,B>;
template<class A, class B> using common2 = typename common2_t<A, B>::type;
static_assert(same<common<int,char,short>, int>(), "common");

template<bool C, class T, class F> using cond_t = std::conditional<C,T,F>;
template<bool C, class T, class F> using cond = typename cond_t<C,T,F>::type;

template<int id, class... Args> struct select_t;
template<int id, class First, class... Rest> struct select_t<id, First, Rest...> : select_t<id-1, Rest...> {};
template<class First, class... Rest> struct select_t<0, First, Rest...> : type_t<First> {};


template<class T, class... Args> struct callable_t {
	template<class U> static true_t f(decltype(declval<U>()(declval<Args>()...))*);
	template<class U> static false_t f(...);
	static const bool value = decltype(f<T>(0))::value;
};
template<class T, class... Args> constexpr bool callable() { return callable_t<T, Args...>::value; }

template<bool ok, class T, class R, class... Args> struct call_convertible_if_callable_t : false_t {};
template<class T, class R, class... Args> struct call_convertible_if_callable_t<true, T, R, Args...> : std::is_convertible<decltype(declval<T>()(declval<Args>()...)), R> {};
template<class T, class... Args> struct call_convertible_if_callable_t<true, T, void, Args...> : true_t {};
template<class T, class R, class... Args> struct call_convertible_args_t : call_convertible_if_callable_t<callable<T, Args...>(), T, R, Args...> {};

template<class Sig, class T> struct call_convertible_t;
template<class T, class R, class... Args> struct call_convertible_t<R(Args...), T> : call_convertible_args_t<T, R, Args...> {};
template<class Sig, class T> constexpr bool call_convertible() { return call_convertible_t<Sig,T>::value; }
template<class Sig, class T> constexpr bool call_convertible(const T &) { return call_convertible_t<Sig,T>::value; }

namespace test_callable {
	void void_void();
	using vv = decltype(void_void);
	int int_int(int);
	using ii = decltype(int_int);
	static_assert(callable<vv>(), "callable");
	static_assert(!callable<vv, bool, bool>(), "callable");
	static_assert(callable<ii, int>(), "callable");
	static_assert(callable<ii, float>(), "callable");
	static_assert(!callable<ii>(), "callable");
	static_assert(!callable<ii, int,int>(), "callable");
	static_assert(!callable<int>(), "callable");
	static_assert(!callable<bool>(), "callable");
	static_assert(call_convertible<void()>(void_void), "callable");
	static_assert(!call_convertible<int()>(void_void), "callable");
	static_assert(!call_convertible<int()>(int_int), "callable");
	static_assert(call_convertible<int(int)>(int_int), "callable");
}




#define TYPEDEF_TEST(_T_) \
	template<class T> struct has_type_ ## _T_ ## _v : type_t<bool> { \
		template<class U> static true_t  f(typename U::_T_ *); \
		template<class U> static false_t f(...); \
		static const bool value = decltype(f<T>(0))::value; \
	}; \
	template<class T> constexpr bool has_type_ ## _T_ () { return has_type_ ## _T_ ## _v<T>::value; }

namespace test_typedef_test {
	struct x { using foo = void; };
	TYPEDEF_TEST(foo);
	static_assert(has_type_foo<x>(), "typedef test");
}

#define CONST_TEST(_F_) \
	template<class T> struct has_const_ ## _F_ ## _v : type_t<bool> { \
		template<class U> static true_t  f(decltype(U::_F_) *); \
		template<class U> static false_t f(...); \
		static const bool value = decltype(f<T>(0))::value; \
	}; \
	template<class T> constexpr bool has_const_ ## _F_ () { return has_const_ ## _F_ ## _v<T>::value; }

namespace test_const_test {
	struct x { static const int bar = 0; };
	CONST_TEST(bar);
	static_assert(has_const_bar<x>(), "const test");
}


TYPEDEF_TEST(nil_t)
template<class T> constexpr bool is_nil() { return has_type_nil_t<T>() && zero_size<T>(); }
namespace test_is_nil {
	static_assert(is_nil<NIL>(), "is nil");
	static_assert(!is_nil<false_t>(), "is nil");
}

TYPEDEF_TEST(type);
CONST_TEST(value);
template<class T, bool v = has_type_type<T>() && has_const_value<T>()> struct is_value_t : false_t {};
template<class T> struct is_value_t<T, true> : same_t<typename T::type, remove_cv<decltype(T::value)>> {};
template<class T> constexpr bool is_value() { return is_value_t<T>::value; }
namespace test_is_value {
	static_assert(has_type_type<true_t>(), "is value");
	static_assert(has_const_value<true_t>(), "is value");
	static_assert(is_value<true_t>(), "is value");
	static_assert(is_value<false_t>(), "is value");
	static_assert(!is_value<NIL>(), "is value");
	struct X { using type = float; static const int value = 3; };
	static_assert(!is_value<X>(), "is value");
}

template<class A, class B> struct same_type_t : same_t<typename A::type, typename B::type> {};
template<class A, class B> constexpr bool same_type() { return same_type_t<A, B>::value; }

template<class A, class B, bool v = is_value<A>() && is_value<B>()> struct same_value_t : false_t {};
template<class A, class B, bool v = same<typename A::type, typename B::type>()> struct same_value_ : false_t {};
template<class A, class B> struct same_value_<A, B, true> : bool_t<A::value == B::value> {};
template<class A, class B> struct same_value_t<A, B, true> : same_value_<A, B> {};
template<class A, class B> constexpr bool same_value() { return same_value_t<A, B>::value; };