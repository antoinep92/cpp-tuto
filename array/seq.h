#pragma once // meta sequence of types and values -- builds on seq_base.h and adds more operations
#include "seq_base.h"
#include "func.h"

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
