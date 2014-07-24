#pragma once // basic introspection, traits and meta data on types and functions
#include "val_base.h"
#include <type_traits>
template<class A, class B> constexpr bool same() { return std::is_same<A,B>::value; }
template<class A, class B> constexpr bool convertible() { return std::is_convertible<A,B>::value; }
template<class T> constexpr bool is_empty() { return std::is_empty<T>::value; }
using std::declval;

template<class... Ts> using common = typename std::common_type<Ts...>::type;
template<class A, class B> using common2_t = std::common_type<A,B>;
template<class A, class B> using common2 = typename std::common_type<A, B>::type;
namespace test_common {
	static_assert(same<common<int,char,short>, int>(), "common");
}

template<class T> using VOID = void;

template<class T, class... Args> struct CALLABLE {
	template<class U> static TRUE f(decltype(declval<U>()(declval<Args>()...))*);
	template<class U> static FALSE f(...);
	static const bool value = decltype(f<T>(0))::value;
};
template<class T, class... Args> constexpr bool callable() { return CALLABLE<T, Args...>::value; }

template<bool ok, class T, class R, class... Args> struct CALL_CONVERTIBLE_ : FALSE {};
template<class T, class R, class... Args> struct CALL_CONVERTIBLE_<true, T, R, Args...> : std::is_convertible<decltype(declval<T>()(declval<Args>()...)), R> {};
template<class T, class... Args> struct CALL_CONVERTIBLE_<true, T, void, Args...> : TRUE {};
template<class T, class R, class... Args> struct CALL_CONVERTIBLE : CALL_CONVERTIBLE_<callable<T, Args...>(), T, R, Args...> {};

template<class Sig, class T> struct CALL_CONVERTIBLE_S;
template<class T, class R, class... Args> struct CALL_CONVERTIBLE_S<R(Args...), T> : CALL_CONVERTIBLE<T, R, Args...> {};
template<class Sig, class T> constexpr bool call_convertible() { return CALL_CONVERTIBLE_S<Sig,T>::value; }
template<class Sig, class T> constexpr bool call_convertible(const T &) { return CALL_CONVERTIBLE_S<Sig,T>::value; }

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
	template<class T> struct has_type_ ## _T_ ## _v : TYPE<bool> { \
		template<class U> static TRUE  f(typename U::_T_ *); \
		template<class U> static FALSE f(...); \
		static const bool value = decltype(f<T>(0))::value; \
	}; \
	template<class T> constexpr bool has_type_ ## _T_ () { return has_type_ ## _T_ ## _v<T>::value; }

namespace test_typedef_test {
	struct x { using foo = void; };
	TYPEDEF_TEST(foo);
	static_assert(has_type_foo<x>(), "typedef_test");
}

#define STATIC_FIELD_TEST(_F_) \
	template<class T> struct has_static_field_ ## _F_ ## _v : TYPE<bool> { \
		template<class U> static TRUE  f(decltype(U::_F_) *); \
		template<class U> static FALSE f(...); \
		static const bool value = decltype(f<T>(0))::value; \
	}; \
	template<class T> constexpr bool has_static_field_ ## _F_ () { return has_static_field_ ## _F_ ## _v<T>::value; }

namespace test_static_field_test {
	struct x { static const int bar = 0; };
	STATIC_FIELD_TEST(bar);
	static_assert(has_static_field_bar<x>(), "static_field_test");
}


TYPEDEF_TEST(nil_t)
template<class T> constexpr bool is_nil() { return has_type_nil_t<T>() && is_empty<T>(); }
namespace test_is_nil {
	static_assert(is_nil<NIL>(), "is nil");
	static_assert(!is_nil<FALSE>(), "is nil");
}

TYPEDEF_TEST(type);
STATIC_FIELD_TEST(value);
template<class T> constexpr bool is_static_value() { return has_type_type<T>() && has_static_field_value<T>(); }
namespace test_is_static_value {
	static_assert(is_static_value<std::true_type>(), "is static value");
	static_assert(is_static_value<FALSE>(), "is static value");
	static_assert(!is_static_value<NIL>(), "is static value");
}