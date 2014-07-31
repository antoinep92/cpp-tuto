#pragma once // meta sequence of types and values
#include "val.h"

template<class... Ts> struct NARGS;
template<> struct NARGS<> : ZERO<size_t> {};
template<class H, class... Ts> struct NARGS<H, Ts...> : inc::val<NARGS<Ts...>> {};
template<class... Ts> constexpr size_t nargs() { return NARGS<Ts...>::value; }

template<class... Ts> struct TYPES { using types = TYPES<Ts...>; };
template<class T, T... args> struct VALUES : TYPE<T> { using values = VALUES<T, args...>; };
TYPEDEF_TEST(isTypes, types)
TYPEDEF_TEST(isValues, values)
template<class T> constexpr bool isList() { return isValues<T>() || isTypes<T>(); }



template<class H, class T> struct CONS_T;
template<class H, class... Ts> struct CONS_T<H, TYPES<Ts...>> : TYPES<H, Ts...> {};
template<class H, class T> using cons_t = typename CONS_T<H, T>::types;

template<class X, X H, class T> struct CONS_V : CONS_V<X, H, typename T::values> {};
template<class X, X H, X... Vs> struct CONS_V<X, H, VALUES<X, Vs...>> : VALUES<X, H, Vs...> {};
template<class X, X H, class T> using cons_v = typename CONS_V<X, H, T>::values;

namespace test_cons {
	static_assert(same<cons_t<int, TYPES<float>>, TYPES<int, float>>(), "cons");
	static_assert(same<cons_v<int, 3, VALUES<int>>, VALUES<int, 3>>(), "cons");
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

//#define MAYBE_TYPE(P_, T_) typename std::enable_if<P_<T>(), T>::type
template<bool C, class T, class F> struct COND : TYPE<T> {};
template<class T, class F> struct COND<false, T, F> : TYPE<F> {};
template<bool C, class T, class F> using cond = typename COND<C, T, F>::type;

namespace test_cond {
	static_assert(cond<true,TRUE,FALSE>::value, "cond");
	static_assert(!cond<false,TRUE,FALSE>::value, "cond");
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

template<int i, bool ok, class T, T... args> struct values_printer {
	static void f(std::ostream & s, const VALUES<T, args...> & vals) {
		if(i) s << ", ";
		s << '$' << at_v<VALUES<T, args...>, i>();
		values_printer<i+1, (i < len<VALUES<T, args...>>()-1), T, args...>::f(s, vals);
	}
};
template<int i, class T, T... args> struct values_printer<i, false, T, args...> {
	static void f(std::ostream & s, const VALUES<T, args...> & vals) {}
};

template<class T, T... args> inline void print_values(std::ostream & s, const VALUES<T, args...> & vals) {
	values_printer<0, (len<VALUES<T, args...>>() > 0), T, args...>::f(s, vals);
};

template<class T, T... args> std::ostream & operator<<(std::ostream & s, const VALUES<T, args...> & vals) {
	s << "values(";
	print_values(s, vals);
	return s << ')';
};

namespace test_values_print {
	t_dyn u1(HERE, []() {
		VALUES<int, 5,3,7> v;
		stringstream ss;
		ss << v;
		return ss.str() == "values($5, $3, $7)";
	});
}

template<class T, template<class> class P> struct COUNT_T;
template<template<class> class P> struct COUNT_T<TYPES<>, P> : ZERO<uint> {};
template<template<class> class P, class F, class... Ns>  struct COUNT_T<TYPES<F, Ns...>, P> : STATIC<uint, (P<F>::value ? 1:0) + COUNT_T<TYPES<Ns...>, P>::value> {};
template<class T, template<class> class P> constexpr typename std::enable_if<isTypes<T>(), uint>::type count_t() { return COUNT_T<T, P>::value; }

template<class T, template<class> class P> struct COUNT_V;
template<class T, template<class> class P> struct COUNT_V<VALUES<T>, P> : ZERO<uint> {};
template<class T, template<class> class P, T F, T... Ns> struct COUNT_V<VALUES<T, F, Ns...>, P> : STATIC<uint, (P<T>::template f<F>::value ? 1:0) + COUNT_V<VALUES<T, Ns...>, P>::value> {};
template<class T, template<class> class P> constexpr typename std::enable_if<isValues<T>(), uint>::type count_v() { return COUNT_V<T, P>::value; }

namespace test_count {
	static_assert(count_t<TYPES<int, char, bool, string, char*>, std::is_integral>() == 3, "count");
	static_assert(count_t<TYPES<int, char, bool, string, char*>, CONST<TRUE>::TF>() == 5, "count");
	static_assert(count_v<VALUES<char, 't','e','s','t'>, BIND_V<EQUALS,char,'t'>::F>() == 2, "count");
	static_assert(count_v<VALUES<char, 't','e','s','t'>, CONST<TRUE>::F>() == 4, "count");
}

template<class T, template<class> class P> struct FILTER_T;
template<template<class> class P> struct FILTER_T<TYPES<>, P> : TYPES<> {};
template<template<class> class P, class F, class... Ns> struct FILTER_T<TYPES<F, Ns...>, P> : COND<
	P<F>::value,
	cons_t< F, typename FILTER_T<TYPES<Ns...>, P>::types >,
	typename FILTER_T<TYPES<Ns...>, P >::types
>::type::types {};
template<class T, template<class> class P> using filter_t = typename FILTER_T<T, P>::types;

template<class T, template<class> class P> struct FILTER_V;
template<class T, template<class> class P> struct FILTER_V<VALUES<T>, P> : VALUES<T> {};
template<class T, template<class> class P, T F, T... Vs> struct FILTER_V<VALUES<T, F, Vs...>, P> : COND<
	P<T>::template f<F>::value,
	cons_v< T, F, typename FILTER_V<VALUES<T, Vs...>, P>::values >,
	typename FILTER_V<VALUES<T, Vs...>, P>::values
>::type::values {};
template<class T, template<class> class P> using filter_v = typename FILTER_V<T, P>::values;

namespace test_filter {
	static_assert(same<filter_t<TYPES<int, string, char, void*, bool, char*>, std::is_integral>, TYPES<int, char, bool>>(), "filter");
	static_assert(same<filter_v<VALUES<int, -2, 5, 3, -1, 5>, POSITIVE>, VALUES<int, 5,3,5>>(), "filter");
}

template<class T, template<class> class P> struct MAP_TT;
template<template<class> class P, class... Ts> struct MAP_TT<TYPES<Ts...>, P> : TYPES< typename P<Ts>::type... > {};
template<class T, template<class> class P> using map_tt = typename MAP_TT<T, P>::types;

template<class T, template<class> class P> struct MAP_TV;
template<template<class> class P, class... Ts> struct MAP_TV<TYPES<Ts...>, P> : VALUES< typename P<void>::type, P<Ts>::value... > {};
template<class T, template<class> class P> using map_tv = typename MAP_TV<T, P>::values;

template<class T, template<class> class P> struct MAP_VT;
template<class T, template<class> class P, T... Vs> struct MAP_VT<VALUES<T, Vs...>, P> : TYPES< typename P<T>::template f<Vs>... > {};
template<class T, template<class> class P> using map_vt = typename MAP_VT<T, P>::types;

template<class T, template<class> class P> struct MAP_VV;
template<class T, template<class> class P, T... Vs> struct MAP_VV<VALUES<T, Vs...>, P> : VALUES< typename P<T>::type, P<T>::template f<Vs>::value... > {};
template<class T, template<class> class P> using map_vv = typename MAP_VV<T, P>::values;

namespace test_map {
	static_assert(same<map_tt<TYPES<bool, float, double>, std::add_const>, TYPES<const bool, const float, const double>>(), "map");
	static_assert(same<map_tv<TYPES<bool, float, double>, SIZEOF>, VALUES<size_t, 1,4,8>>(), "map");
	static_assert(same< map_vt<VALUES<int, 1,3,5>, MAKE_VALUE>, TYPES<STATIC<int,1>, STATIC<int,3>, STATIC<int,5>> >(), "map");
	static_assert(same< map_vv<VALUES<int, 1,3,5>, INC>, VALUES<int, 2,4,6>>(), "map");
}

template<class T, template<class,class> class B, class S = void> struct REDUCE_TT;
template<template<class,class> class B, class S> struct REDUCE_TT<TYPES<>, B, S> : TYPE<S> {};
template<template<class,class> class B, class S, class F, class... Ns> struct REDUCE_TT<TYPES<F, Ns...>, B, S> : B<F, typename REDUCE_TT<TYPES<Ns...>, B, S>::type> {};
template<class T, template<class,class> class B, class S = void> using reduce_tt = typename REDUCE_TT<T, B, S>::type;

template<class T, template<class> class B, typename T::type S = B<typename T::type>::value> struct REDUCE_VV;
template<class T, template<class> class B, T S> struct REDUCE_VV<VALUES<T>, B, S> : STATIC<T,S> {};
template<class T, template<class> class B, T S, T F, T... Ns> struct REDUCE_VV<VALUES<T, F, Ns...>, B, S> : B<T>::template bin<F, REDUCE_VV<VALUES<T, Ns...>, B, S>::value> {};
template<class T, template<class> class B, typename T::type S = B<typename T::type>::value> constexpr typename T::type reduce_vv() { return REDUCE_VV<T, B, S>::value; }

namespace test_reduce {
	static_assert(same<reduce_tt<TYPES<int, char, float, double>, COMMON2, bool>, double>(), "reduce");
	static_assert(reduce_vv<VALUES<int, 1,2,3>, ADD, 10>() == 16, "reduce");
	static_assert(reduce_vv<VALUES<int, 3,4,10>, MUL>() == 120, "reduce");
}

template<class T, template<class> class B, typename T::type S = B<typename T::type>::value> struct CUMUL;
template<class T, template<class> class B, T S> struct CUMUL<VALUES<T>, B, S> : VALUES<T> {};
template<class T, template<class> class B, T S, T F, T... Ns> struct CUMUL<VALUES<T, F, Ns...>, B, S> : CONS_V<T, B<T>::template bin<F,S>::value, CUMUL<VALUES<T, Ns...>, B, B<T>::template bin<F,S>::value>> {};
template<class T, template<class> class B, typename T::type S = B<typename T::type>::value> using cumul = typename CUMUL<T, B, S>::values;

namespace test_cumul {
	static_assert(same< cumul<VALUES<int, 1,2,3,4,5>, ADD>, VALUES<int, 1,3,6,10,15> >(), "cumul");
	static_assert(same< cumul<VALUES<int, 1,2,3,4,5>, MUL>, VALUES<int, 1,2,6,24,120> >(), "cumul");
}

template<class Vals, class Target> struct CAST_V;
template<class T, class Target, T... Vals> struct CAST_V<VALUES<T, Vals...>, Target> : VALUES<Target, Target(Vals)...> {};
template<class Vals, class Target> using cast_v = typename CAST_V<Vals, Target>::values;





template<int... args> struct ints {
	using ARGS = VALUES<int, args...>;
	static const uint n = len<ARGS>();
	using DYN = map_vv<ARGS, NEGATIVE>;
	static const uint n_static = count_v<DYN, NOT>(), n_dynamic = n - n_static;
	using INDEX = map_vv< cumul<cast_v<DYN, int>, ADD>, DEC>;
	uint data[n_dynamic];
	
	template<int i> int read() const {
		return at_v<DYN,i>() ? data[at_v<INDEX,i>()] : at_v<ARGS,i>();
	}
	template<int i> uint & write() {
		if(at_v<DYN,i>()) return data[at_v<INDEX,i>()];
		else throw "bad";
	}
	
	ints() : data() {}
};

template<int i, bool ok, int... args> struct ints_printer {
	static void f(std::ostream & s, const ints<args...> & vals) {
		if(i) s << ", ";
		if(!at_v<typename ints<args...>::DYN,i>()) s << '$';
		s << vals.template read<i>();
		ints_printer<i+1, (i < ints<args...>::n-1), args...>::f(s, vals);
	}
};
template<int i, int... args> struct ints_printer<i, false, args...> {
	static void f(std::ostream & s, const ints<args...> & vals) {}
};

template<int... args> inline void print_ints(std::ostream & s, const ints<args...> & vals) {
	ints_printer<0, (ints<args...>::n > 0), args...>::f(s, vals);
};

template<int... args> std::ostream & operator<<(std::ostream & s, const ints<args...> & vals) {
	s << "ints(";
	print_ints(s, vals);
	return s << ')';
};

namespace ints_test {
	template<int i> struct read {
		template<int... Vs> static int f(const ints<Vs...> & vs) { return vs.template read<i>(); }
	};
	using V = ints<-1, 1, 2, -1, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15>;
	static_assert(V::n == 16, "ints");
	static_assert(V::n_static == 14, "ints");
	static_assert(V::n_dynamic == 2, "ints");
	static t_dyn u1(HERE, []() {
		V v;
		v.write<3>() = 3;
		for(int i = 0; i < 16; ++i)
			if(dispatch_linear<int,read,0,16>(i,v) != i) return false;
		return true;
	});
	static t_dyn u2(HERE, []() {
		V v; v.write<3>() = 3;
		stringstream ss;
		ss << v;
		return ss.str() == "ints(0, $1, $2, 3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15)";
	});
}
