#include <iostream>
using std::cout; using std::cerr; using std::endl;
#include <tuple>

/*	conventions

		lowercase or CamelCase	first-class object
		UPPERCASE				type which *inderectly* represents a first-class object
		
			
			using X = STATIC<int, 3>
				X represents 3 (X::value). Indirection => uppercase
			
			static const int x = 3; 
				x is directly 3 => No indirection, lowercase
				
			using T = MAKE_CONST<int>
				T represents const int (T::type). Indirection => uppercase
			
			using T = make_cast<int>
				T is an alias to const int. No indirection => lowercase
			
			struct EMPTY {}
				EMPTY is used as a tag, to represent an empty set. The empty set is not even readable, only a convention => uppercase
*/

using uint = unsigned int;

template<class T> struct TYPE { using type = T; };

template<class A, class B> using common2_tt = typename std::common_type<A, B>::type;
template<class... Ts> struct COMMON;
template<class T> struct COMMON<T> : TYPE<T> {};
template<class F, class... Ns> struct COMMON<F, Ns...> : std::common_type<F, typename COMMON<Ns...>::type> {};
template<class... Ts> using common_tt = typename COMMON<Ts...>::type;


enum ValueKind { sta, dyn, ref };

template<ValueKind K> struct VALUE_KIND {
	using value_flag = void;
	static const ValueKind kind = K;
	static const bool isStatic = (K == sta);
	static const bool isDynamic = !isStatic;
};
template<class T, ValueKind SD = dyn, T V = T()> struct VALUE;

template<class T, T V> struct VALUE<T, sta, V> : TYPE<T>, VALUE_KIND<sta> {
	static const T value = V;
	operator const T & () const { return value; }
};

template<class T, T V> struct VALUE<T, dyn, V> : TYPE<T>, VALUE_KIND<dyn> {
	T value;
	operator const T & () const { return value; }
	operator T & () { return value; }
	template<class U> T & operator=(const U & v) { return value = v; }
};

template<class T, T V> struct VALUE<T, ref, V> : TYPE<T>, VALUE_KIND<ref> {
	const T & value;
	operator const T & () const { return value; }
};

template<class T, T V> using STATIC = VALUE<T, sta, V>;
template<class T> using DYNAMIC = VALUE<T, dyn>; 
template<class T> using REF = VALUE<T, ref>;

using YES = void*;
using NO = bool;

template<class T> using ZERO = STATIC<T, 0>;
template<class T> using ONE = STATIC<T, 1>;
using TRUE = STATIC<bool, true>;
using FALSE = STATIC<bool, false>;
template<class A, class B> using ADD = STATIC<COMMON<typename A::type, typename B::type>, A::value + B::value>;
template<class X> using INC = STATIC<typename X::type, 1 + X::value>;

struct NIL { using nil_tag = void; };

#define TYPEDEF_TEST(fName_, typeName_) \
	template<class T> struct fName_ ## _ : FALSE { \
		template<class U> static YES f(typename U::typeName_ *); \
		template<class U> static NO f(...); \
		static const bool value = (sizeof(YES) == sizeof(f<T>(0))); \
	}; \
	template<class T> constexpr bool fName_() { return fName_ ## _<T>::value; } \
	template<class T> constexpr bool fName_(const T &) { return fName_ ## _<T>::value; }

TYPEDEF_TEST(isNil, nil_tag)
TYPEDEF_TEST(isValue, value_tag)

template<class... Ts> struct TYPES { using types = TYPES<Ts...>; };
TYPEDEF_TEST(isTypes, types)

template<class H, class T> struct CONS_T;
template<class H, class... Ts> struct CONS_T<H, TYPES<Ts...>> : TYPES<H, Ts...> {};
template<class H, class T> using cons_t = typename CONS_T<H, T>::types;

template<class A, class B> struct CAT_T;
template<class... As, class... Bs> struct CAT_T<TYPES<As...>, TYPES<Bs...>> : TYPES<As..., Bs...> {};
template<class A, class B> using cat_t = typename CAT_T<A, B>::types;

template<class T, T... args> struct VALUES : TYPE<T> { using values = VALUES; };
TYPEDEF_TEST(isValues, values)

template<class X, X H, class T> struct CONS_V;
template<class X, X H, X... Vs> struct CONS_V<X, H, VALUES<X, Vs...>> : VALUES<X, H, Vs...> {};
template<class X, X H, class T> using cons_v = typename CONS_V<X, H, T>::values;

template<class A, class B> struct CAT_V;
template<class X, X... As, X... Bs> struct CAT_V<VALUES<X, As...>, VALUES<X, Bs...>> {};
template<class A, class B> using cat_v = typename CAT_V<A, B>::values;

template<class T> constexpr bool isList() { return isValues<T>() || isTypes<T>(); }

//#define MAYBE_TYPE(P_, T_) typename std::enable_if<P_<T>(), T>::type
template<bool C, class T, class F> struct COND : TYPE<T> {};
template<class T, class F> struct COND<false, T, F> : TYPE<F> {};
template<bool C, class T, class F> using cond = typename COND<C, T, F>::type;

template<class T> struct LEN;
template<> struct LEN<TYPES<>> : ZERO<uint> {};
template<class T> struct LEN<VALUES<T>> : ZERO<uint> {};
template<class F, class... Ns> struct LEN<TYPES<F, Ns...>> : INC<LEN<TYPES<Ns...>>> {};
template<class T, T F, T... Ns> struct LEN<VALUES<T, F, Ns...>> : INC<LEN<VALUES<T, Ns...>>> {};
template<class T> constexpr typename std::enable_if<isList<T>(), uint>::type len() { return LEN<T>::value; }

template<class T, template<class> class P> struct COUNT_T;
template<template<class> class P> struct COUNT_T<TYPES<>, P> : ZERO<uint> {};
template<template<class> class P, class F, class... Ns>  struct COUNT_T<TYPES<F, Ns...>, P> : STATIC<uint, (P<F>::value ? 1:0) + COUNT_T<TYPES<Ns...>, P>::value> {};
template<class T, template<class> class P> constexpr typename std::enable_if<isList<T>(), uint>::type count() { return COUNT_T<T, P>::value; }

template<class T, template<typename T::type> class P> struct COUNT_V;
template<class T, template<T> class P> struct COUNT_V<VALUES<T>, P> : ZERO<uint> {};
template<class T, template<T> class P, T F, T... Ns> struct COUNT_V<VALUES<T, F, Ns...>, P> : STATIC<uint, (P<F>::value ? 1:0) + COUNT_V<VALUES<T, Ns...>, P>::value> {};
template<class T, template<typename T::type> class P> constexpr typename std::enable_if<isList<T>(), uint>::type count() { return COUNT_V<T, P>::value; }

template<class T, template<class> class P> struct FILTER_T;
template<template<class> class P> struct FILTER_T<TYPES<>, P> : TYPES<> {};
template<template<class> class P, class F, class... Ns> struct FILTER_T<TYPES<F, Ns...>, P> : COND<
	P<F>::value,
	typename CONS_T< F, typename FILTER_T<TYPES<Ns...>, P>::types >::type,
	typename FILTER_T<TYPES<Ns...>, P >::types
>::type::types {};
template<class T, template<class> class P> using filter_t = typename FILTER_T<T, P>::types;

template<class T, template<typename T::type> class P> struct FILTER_V;
template<class T, template<T> class P> struct FILTER_V<VALUES<T>, P> : VALUES<T> {};
template<class T, template<T> class P, T F, T... Vs> struct FILTER_V<VALUES<T, F, Vs...>, P> : COND<
	P<F>::value,
	typename CONS_V< T, F, typename FILTER_V<VALUES<T, Vs...>, P>::values >::type,
	typename FILTER_V<VALUES<T, Vs...>, P>::values
>::type::values {};
template<class T, template<typename T::type> class P> using filter_v = typename FILTER_V<T, P>::values;

template<class T, template<class> class P> struct MAP_TT;
template<template<class> class P, class... Ts> struct MAP_TT<TYPES<Ts...>, P> : TYPES< typename P<Ts>::type... > {};
template<class T, template<class> class P> using map_tt = typename MAP_TT<T, P>::types;

template<class T, template<class> class P> struct MAP_TV;
template<template<class> class P, class... Ts> struct MAP_TV<TYPES<Ts...>, P> : VALUES< typename P<void>::type, P<Ts>::value... > {};
template<class T, template<class> class P> using map_tv = typename MAP_TV<T, P>::values;

template<class T, template<typename T::type> class P> struct MAP_VT;
template<class T, template<typename T::type> class P, T... Vs> struct MAP_VT<VALUES<T, Vs...>, P> : TYPES< typename P<Vs>::type... > {};
template<class T, template<typename T::type> class P> using map_vt = typename MAP_VT<T, P>::types;

template<class T, template<typename T::type> class P> struct MAP_VV;
template<class T, template<typename T::type> class P, T... Vs> struct MAP_VV<VALUES<T, Vs...>, P> : VALUES< typename P<std::declval<T>()>::type, P<Vs>::value... > {};
template<class T, template<typename T::type> class P> using map_vv = typename MAP_VV<T, P>::values;

template<class T, template<class,class> class B, class S = void> struct REDUCE_TT;
template<template<class,class> class B, class S> struct REDUCE_TT<TYPES<>, B, S> : TYPE<S> {};
template<template<class,class> class B, class S, class F, class... Ns> struct REDUCE_TT<TYPES<F, Ns...>, B, S> : B<F, typename REDUCE_TT<TYPES<Ns...>, B, S>::type> {};
template<class T, template<class,class> class B, class S = void> using reduce_tt = typename REDUCE_TT<T, B, S>::type;

template<class T, template<typename T::type, typename T::type> class B, typename T::type S = typename T::type()> struct REDUCE_VV;
template<class T, template<T,T> class B, T S> struct REDUCE_VV<VALUES<T>, B, S> : VALUE<T,S> {};
template<class T, template<T,T> class B, T S, T F, T... Ns> struct REDUCE_VV<VALUES<T, F, Ns...>, B, S> : B<F, REDUCE_VV<VALUES<T, Ns...>, B, S>::value> {};
template<class T, template<typename T::type, typename T::type> class B, typename T::type S = typename T::type()> constexpr typename T::type reduce_vv() { return REDUCE_VV<T, B, S>::value; }


template<class T, ValueKind SD, T V> std::ostream & operator<<(std::ostream & s, const VALUE<T, SD, V> & val) {
	if(SD == sta) s << '$';
	return s << val.value;
}



template<int... args> using ints_t = std::tuple< VALUE<uint, args < 0 ? dyn : sta, args < 0 ? 0 : args>... >;

template<int I, int N> struct tuple_ostream {
	template<class... Ts> static void f(std::ostream & s, const std::tuple<Ts...> & tp) {
		if(I) s << ", ";
		s << std::get<I>(tp);
		tuple_ostream<I+1, N>::f(s, tp);
	}
};
template<int N> struct tuple_ostream<N,N> {
	template<class... Ts> static void f(std::ostream &, const std::tuple<Ts...> &) {}
};
template<class... Ts> std::ostream & operator<<(std::ostream & s, const std::tuple<Ts...> & tp) {
	s << "tuple(";
	tuple_ostream<0, count<TYPES<Ts...>>()>::f(s, tp);
	return s << ')';
}

template<class T> struct POSITIVE {
	template<T x> using pred = STATIC<bool, (x >= 0)>;
};
template<class T> struct NEGATIVE {
	template<T x> using pred = STATIC<bool, (x < 0)>;
};
template<class T> struct SUM {
	template<T a, T b> using bin = STATIC<T, a+b>;
};

template<int... args_> struct ints {
	using args = VALUES<int, args_...>;
	static const uint n = count<VALUES<int, args_...>>();
	static const uint n_static = count<VALUES<int, args_...>, POSITIVE<int>::pred>(), n_dynamic = n - n_static;
	using dyn = typename MAP_VV<VALUES<int, args_...>, NEGATIVE<int>::pred>::values;
	using index = typename CUMSUM<dyn>::values;
	uint data[n_dynamic];
	
	template<int i> int read() const {
		return AT<dyn, i>::value ? data[AT<index, i>::value] : AT<args, i>::value;
	}
	q
};


int main() {
	ints_t<-1, 1, 2, -1, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15> vt;
	std::get<3>(vt) = 3;
	cout << sizeof(vt) << endl;
	cout << vt << endl;
	
	ints<-1, 1, 2, -1, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15> v;
	cout << sizeof(v) << endl;
	cout << v << endl;
	return 0;
}
