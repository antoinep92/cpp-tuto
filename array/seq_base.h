#pragma once // meta sequence of types and values -- basic constructs
#include "val.h"

// argument counters
template<class... Ts> struct countTypes_t;
template<> struct countTypes_t<> : zero_t<int> {};
template<class H, class... Ts> struct countTypes_t<H, Ts...> : inc::val<countTypes_t<Ts...>> {};
template<class... Ts> constexpr int countTypes() { return countTypes_t<Ts...>::value; }

template<class T, T Vs...> struct countValues_t;
template<class T> struct countValues_t<> : zero_t<int> {};
template<class T, T h, T Vs...> struct countValues_t<T, h, Vs...> : inc::val<countValues_t<T, Vs...>> {};
template<class T, T... Vs> constexpr int countValues() { return countValues_t<T, Vs...>::value; }

// containers
template<class... Types> struct types_t {
	using types_base = type_t<Types...>;
	static const int length = countTypes<Types...>();
};
template<class T, T... values> struct values_t : type_t<T> {
	using values_base = values_t<T, values...>;
	static const int length = countValues<T, values...>();
};

// tester traits
TYPEDEF_TEST(types_base)
template<class T> using is_types_t = has_types_type_t<T>;
GEN_CXPR1(is_types)

TYPEDEF_TEST(values_vase)
template<class T> using is_values_t = has_values_type_t<T>;
GEN_CXPR1(is_values)

template<class T> using is_list_t = lor::val<is_types_t<T>, is_values_t<T>>;
GEN_CXPR1(is_list)


template<class Head, class Types> struct consT_;
template<class Head, class... Types> struct consT_<Head, types_t<Types...>> : types_t<Head, Types...> {};
template<class Head, class Tail> using consT_t = consT_<Head, typename Tail::types_base>;
template<class Head, class Tail> using consT = typename consT_t<Head, Tail>::types;

template<class X, X head, class Tail> struct consV_;
template<class X, X head, X... values> struct consV_<X, head, values_t<X, values...>> : values_t<X, head, values...> {};
template<class X, X head, class Tail> struct consV_t = consV_<X, head, typename Tail::values_base>;
template<class X, X head, class Tail> using consV = typename consV_t<X, head, Tail>::values;

namespace test_cons {
	static_assert(same<consT<int, types_t<float>>, types_t<int, float>>(), "cons");
	static_assert(same<consV<int, 3, values_t<int>>, values_t<int, 3>>(), "cons");
}

template<class A, class B> struct CAT_T;
template<class... As, class... Bs> struct CAT_T<TYPES<As...>, TYPES<Bs...>> : TYPES<As..., Bs...> {};
template<class A, class B> using cat_t = typename CAT_T<A, B>::types;

template<class A, class B> struct CAT_V;
template<class X, X... As, X... Bs> struct CAT_V<VALUES<X, As...>, VALUES<X, Bs...>> : VALUES<X, As..., Bs...> {};
template<class A, class B> using cat_v = typename CAT_V<A, B>::values;

namespace test_cat {
	static_assert(same<cat_t<TYPES<void, int>, TYPES<float, double>>, TYPES<void, int, float, double>>(), "cat");
	static_assert(same<cat_v<VALUES<int, 1,2>, VALUES<int, 3,4>>, VALUES<int, 1,2,3,4>>(), "cat");
}

template<class L, int I> struct AT;
template<class F, class... Ns> struct AT<TYPES<F, Ns...>, 0> : TYPE<F> {};
template<int I, class F, class... Ns> struct AT<TYPES<F, Ns...>, I> : AT<TYPES<Ns...>, I-1> { static_assert(I >= 0, "out of bounds"); };
template<class T, T F, T... Ns> struct AT<VALUES<T, F, Ns...>, 0> : STATIC<T, F> {};
template<int I, class T, T F, T... Ns> struct AT<VALUES<T, F, Ns...>, I> : AT<VALUES<T,Ns...>, I-1> { static_assert(I >= 0, "out of bounds"); };
template<class T, int i> using at_t = typename AT<T, i>::type;
template<class T, int i> constexpr typename T::type at_v() { return AT<T,i>::value; }

namespace test_at {
	static_assert(same<at_t<TYPES<void,float>,0>,void>(), "at");
	static_assert(same<at_t<TYPES<void,float>,1>,float>(), "at");
	static_assert(at_v<VALUES<int,1,3,5>,0>() == 1, "at");
	static_assert(at_v<VALUES<int,1,3,5>,1>() == 3, "at");
	static_assert(at_v<VALUES<int,1,3,5>,2>() == 5, "at");
}


template<class T> struct LEN;
template<> struct LEN<TYPES<>> : ZERO<uint> {};
template<class T> struct LEN<VALUES<T>> : ZERO<uint> {};
template<class F, class... Ns> struct LEN<TYPES<F, Ns...>> : inc<LEN<TYPES<Ns...>>> {};
template<class T, T F, T... Ns> struct LEN<VALUES<T, F, Ns...>> : inc<LEN<VALUES<T, Ns...>>> {};
template<class T> constexpr typename std::enable_if<isList<T>(), uint>::type len() { return LEN<T>::value; }

namespace test_len {
	static_assert(len<TYPES<>>() == 0, "len");
	static_assert(len<TYPES<int, double>>() == 2, "len");
	static_assert(len<VALUES<bool>>() == 0, "len");
	static_assert(len<VALUES<char, 't','e','s','t'>>() == 4, "len");
}