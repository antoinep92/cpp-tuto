#pragma once // basic introspection, traits and meta data on types and functions
#include <type_traits>
using TRUE = std::true_type;
using FALSE = std::false_type;
template<class A, class B> constexpr bool same() { return std::is_same<A,B>::value; }
template<class A, class B> constexpr bool convertible() { return std::is_convertible<A,B>::value; }
using std::declval;

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