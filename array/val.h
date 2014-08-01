#pragma once // basic containers for types and (integral typed) values -- builds on val_base.h with extra dependencies
#include "val_base.h"
#include "traits.h"
#include "test.h"
#include <iostream>



template<class T, value_kind SD, T V> std::ostream & operator<<(std::ostream & s, const VALUE<T, SD, V> & val) {
	if(SD == sta) s << '$';
	return s << val.value;
}

namespace test_value {
	static t_dyn u2(HERE, dynamic_t<int>(5).value == 5);
	static t_dyn u3(HERE, []() { dynamic_t<char> v; v = 'a'; return v == 'a'; });
}
namespace test_value_print {
	t_dyn u1(HERE, []() {
		static_t<int,5> s;
		dynamic_t<int> d(3);
		std::stringstream ss;
		ss << s << ' ' << d;
		return ss.str() == "$5 3";
	});
}

struct NIL { using nil_t = NIL; };
TYPEDEF_TEST(nil_t)
template<class T> constexpr bool is_nil() { return has_type_nil_t<T>() && zero_size<T>(); }
namespace test_is_nil {
	static_assert(is_nil<NIL>(), "is nil");
	static_assert(!is_nil<false_t>(), "is nil");
}


TYPEDEF_TEST(type);
template<class T> using is_type_t = has_type_type_t<T>;
template<class T> constexpr bool is_type() { return is_type_t<T>::value; }

CONST_TEST(value);
template<class T, bool v = is_type<T>() && has_const_value<T>()> struct is_value_t : false_t {};
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

template<class A, class B, bool v = is_type<A>() && is_type<B>()> struct same_type_t : false_t {};
template<class A, class B> struct same_type_t<A, B, true> : same_t<typename A::type, typename B::type> {};
template<class A, class B> constexpr bool same_type() { return same_type_t<A, B>::value; }
namespace test_same_type {
	static_assert(same_type<type_t<int>, static_t<int, 3>>(), "same type");
	static_assert(!same_type<int, type_t<bool>>(), "same type");
}

template<class A, class B, bool v = is_value<A>() && is_value<B>()> struct same_value_t : false_t {};
template<class A, class B, bool v = same<typename A::type, typename B::type>()> struct same_value_ : false_t {};
template<class A, class B> struct same_value_<A, B, true> : bool_t<A::value == B::value> {};
template<class A, class B> struct same_value_t<A, B, true> : same_value_<A, B> {};
template<class A, class B> constexpr bool same_value() { return same_value_t<A, B>::value; };
namespace test_same_value {
	struct one : one_t<int> {};
	static_assert(same_value<one_t<int>, one>(), "same value");
	static_assert(!same_value<one, zero_t<int>>(), "same value");
	static_assert(!same_value<one, one_t<char>>(), "same value");
	static_assert(!same_value<one, true_t>(), "same value");
	static_assert(!same_value<one, type_t<int>>(), "same value");
	static_assert(!same_value<one, int>(), "same value");
}