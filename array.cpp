#include <iostream>
using std::cout; using std::cerr; using std::endl;
#include <string>
using std::string;
#include <sstream>
using std::stringstream;
#include <tuple>
using std::tuple;
#include <exception>
using std::exception;
#include <functional>
using std::enable_if; using std::declval;

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

using YES = void*;
using NO = bool;
namespace test_yesno {
	static_assert(sizeof(YES) != sizeof(NO), "yesno");
}

using uint = unsigned int;

template<class T> using VOID = void;
template<class A, class B> constexpr bool convertible() { return std::is_convertible<A,B>::value; }

template<class T> struct TYPE { using type = T; };

enum ValueKind { sta, dyn, ref };

template<ValueKind K> struct VALUE_KIND {
	using value_tag = void;
	static const ValueKind kind = K;
	static const bool isStatic = (K == sta);
	static const bool isDynamic = !isStatic;
};
template<class T, ValueKind SD = dyn, T V = T()> struct VALUE;

template<class T, T V> struct VALUE<T, sta, V> : TYPE<T>, VALUE_KIND<sta> {
	static const T value = V;
	operator T () const { return value; }
};

template<class T, T V> struct VALUE<T, dyn, V> : TYPE<T>, VALUE_KIND<dyn> {
	T value;
	operator T () const { return value; }
	operator T & () { return value; }
	template<class U> T & operator=(const U & v) { return value = v; }
	VALUE(T v = T()) : value(v) {}
};

template<class T, T V> struct VALUE<T, ref, V> : TYPE<T>, VALUE_KIND<ref> {
	const T & value;
	operator const T & () const { return value; }
};

template<class T, T V> using STATIC = VALUE<T, sta, V>;
template<class T> using DYNAMIC = VALUE<T, dyn>; 
template<class T> using REF = VALUE<T, ref>;

namespace test_value {
	static_assert(STATIC<int,3>::value == 3, "value");
}

template<class T, ValueKind SD, T V> std::ostream & operator<<(std::ostream & s, const VALUE<T, SD, V> & val) {
	if(SD == sta) s << '$';
	return s << val.value;
}

template<class T> using ZERO = STATIC<T, T(0)>;
template<class T> using ONE = STATIC<T, T(1)>;
template<class T> using MONE = STATIC<T, T(-1)>;
using TRUE = STATIC<bool, true>;
using FALSE = STATIC<bool, false>;

template<class T> struct MAKE_VALUE {
	template<T x> using f = STATIC<T,x>;
};


template<class A, class B> struct SAME : FALSE {};
template<class T> struct SAME<T,T> : TRUE {};
template<class A, class B> constexpr bool same() { return SAME<A,B>::value; }

namespace test_same {
	static_assert(same<bool, bool>(), "same");
	static_assert(same<TYPE<int>::type, int>(), "same");
}

template<class... Ts> using common = typename std::common_type<Ts...>::type;
template<class A, class B> using COMMON2 = std::common_type<A,B>;
template<class A, class B> using common2 = typename std::common_type<A, B>::type;
namespace test_common {
	static_assert(same<common<int,char,short>, int>(), "common");
}


template<class A, class B> using add = STATIC<common<typename A::type, typename B::type>, A::value + B::value>;
template<class X> using inc = STATIC<typename X::type, 1 + X::value>;

namespace test_valop {
	static_assert(inc<ZERO<int>>::value == 1, "valop");
	static_assert(add<STATIC<int, 3>, STATIC<int, 10>>::value == 13, "valop");
}



template<class T, class... Args> struct CALLABLE {
	template<class U> static YES f(decltype(declval<U>()(declval<Args>()...))*);
	template<class U> static NO f(...);
	static const bool value = (sizeof(f<T>(0)) == sizeof(YES));
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


struct SourceContext {
	string file;
	int line;
	operator string () const {
		stringstream ss;
		ss << file << ':' << line;
		return ss.str();
	}
};

struct E : exception {
	const string msg;
	E(const string & msg) : msg(msg) {}
	const char* what() const noexcept { return msg.c_str(); }
};
struct ELocalizedException : E {
	template<class... Args> ELocalizedException(const SourceContext & context, const Args & ... args) : E(format(context, args...)) {}
	ELocalizedException(const std::string & desc) : E(desc) {}
	template<class S> static void print_lines(S &) {}
	template<class S, class F, class... Args> static void print_lines(S & s, const F & f, const Args & ... args) {
		s << f << '\n';
		print_lines(s, args...);
	}
	template<class... Args> static std::string format(const SourceContext & context, const Args & ... args) {
		stringstream ss;
		ss << "Exception at " << string(context) << '\n';
		print_lines(ss, args...);
		return ss.str();
	}
	
};

struct EUnitFail : ELocalizedException {
	template<class... Args> EUnitFail(const SourceContext & context, const Args & ... args) : ELocalizedException(context, "Unit test failed", args...) {}
};

template<bool b> struct t_sta;
template<> struct t_sta<true> {};

#define HERE SourceContext{__FILE__, __LINE__}

struct t_dyn {
	template<class B> static typename enable_if<!callable<B>() && convertible<B,bool>()>::type test(const SourceContext & context, const B & b) {
		if(!b) throw EUnitFail(context, "expression is false");
	}
	template<class F> static typename enable_if<call_convertible<void(), F>() && !call_convertible<bool(), F>()>::type test(const SourceContext & context, const F & f) {
		try {
			f();
		} catch(std::exception & e) {
			throw EUnitFail(context, e.what());
		} catch(...) {
			throw EUnitFail(context, "non-exception thrown");
		}
	}
	template<class F> static typename enable_if<call_convertible<bool(), F>()>::type test(const SourceContext & context, const F & f) {
		try {
			if(!f()) throw ELocalizedException(HERE, "unit test's return value evalutes to false");
		} catch(std::exception & e) {
			throw EUnitFail(context, e.what());
		} catch(...) {
			throw EUnitFail(context, "non-exception thrown");
		}
	}
	template<class T> t_dyn(const SourceContext & context, const T & expr) {
		test(context, expr);
	}
};

namespace test_value {
	static t_dyn u2(HERE, DYNAMIC<int>(5).value == 5);
	static t_dyn u3(HERE, []() { DYNAMIC<char> v; v = 'a'; return v == 'a'; });
}
namespace test_value_print {
	t_dyn u1(HERE, []() {
		STATIC<int,5> s;
		DYNAMIC<int> d(3);
		stringstream ss;
		ss << s << ' ' << d;
		return ss.str() == "$5 3";
	});
}

template<class... Ts> struct NARGS;
template<> struct NARGS<> : ZERO<uint> {};
template<class H, class... Ts> struct NARGS<H, Ts...> : inc<NARGS<Ts...>> {};
template<class... Ts> constexpr uint nargs() { return NARGS<Ts...>::value; }

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
	tuple_ostream<0, nargs<Ts...>()>::f(s, tp);
	return s << ')';
}

namespace tuple_print_test {
	static t_dyn u1(HERE, []() {
		std::tuple<int, char, string> v(3, 'a', "test");
		stringstream ss;
		ss << v;
		return ss.str() == "tuple(3, a, test)";
	});
}

template<class In, template<In> class F, In begin, In end, In cur = begin> struct DISPATCH_LINEAR {
	template<class... Args> static decltype(F<cur>::f(declval<Args>()...)) f(In dyn, Args && ... args) {
		if(cur == dyn) return F<cur>::f(args...);
		else return DISPATCH_LINEAR<In, F, begin,end,cur+1>::f(dyn, args...);
	}
};
template<class In, template<In> class F, In begin, In end> struct DISPATCH_LINEAR<In, F, begin, end, end> {
	template<class... Args> static decltype(F<begin>::f(declval<Args>()...)) f(In dyn, Args && ... args) {
		throw E("out of bounds in linear dispatch");
	}
};
template<class In, template<In> class F, In begin, In end, class... Args> inline decltype(F<begin>::f(declval<Args>()...)) dispatch_linear(In dyn, Args && ... args) {
	return DISPATCH_LINEAR<In, F, begin, end>::f(dyn, args...);
}

namespace ints_tuple_test {
	template<int i> struct get_tuple {
		template<class... Ts> static int f(const std::tuple<Ts...> & t) { return std::get<i>(t); }
	};
	using V = ints_t<-1, 1, 2, -1, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15>;
	static t_dyn u1(HERE, []() {
		V v; std::get<3>(v) = 3;
		for(int i = 0; i < 16; ++i)
			if(dispatch_linear<int,get_tuple,0,16>(i,v) != i) return false;
		return true;
	});
	static t_dyn u2(HERE, []() {
		V v; std::get<3>(v) = 3;
		stringstream ss;
		ss << v;
		return ss.str() == "tuple(0, $1, $2, 3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15)";
	});
}


#define MAKE_OP(_NAME_, _OP_, _OUT_, _INIT_) \
	template<class In> struct _NAME_ : STATIC<_OUT_, _INIT_> { \
		template<In a, In b> using bin = STATIC<_OUT_, (a _OP_ b)>; \
	}
MAKE_OP(ADD, +, In, 0);
MAKE_OP(MUL, *, In, 1);

MAKE_OP(EQUALS, ==, bool, false);
MAKE_OP(NEQ, !=, bool, false);
MAKE_OP(LESS, <, bool, false);
MAKE_OP(GREATER, >, bool, false);
MAKE_OP(LEQ, <=, bool, false);
MAKE_OP(GEQ, >=, bool, false);

namespace test_bin {
	static_assert(ADD<int>::bin<3,2>::value == 5, "bin");
	static_assert(MUL<int>::bin<3,2>::value == 6, "bin");
	static_assert(EQUALS<char>::bin<'a', 'a'>::value, "bin");
}

template<template<class> class Bin, template<class> class Val> struct BIND_P {
	template<class T> struct F : TYPE<T> {
		template<T x> using f = STATIC<T, Bin<T>::template bin<x, Val<T>::value>::value>;
	};
};

template<template<class> class Bin, class X, X V> struct BIND_V {
	template<class T> struct F : TYPE<T> {
		template<T x> using f = STATIC<T, Bin<T>::template bin<x, T(V)>::value>;
	};
};

namespace test_bind {
	static_assert(BIND_P<ADD, ONE>::F<int>::f<2>::value == 3, "bind");
	static_assert(BIND_V<MUL, int, 2>::F<int>::f<3>::value == 6, "bind");
}

template<class T> struct IDENTITY : TYPE<T> {
	template<T x> using f = STATIC<T, x>;
};
template<class T> struct NOT : TYPE<bool> {
	template<T x> using f = STATIC<T, !x>;
};
template<class T> struct SIZEOF : STATIC<size_t, sizeof(T)> {};
template<> struct SIZEOF<void> : STATIC<size_t, 0> {};
template<class T> using POSITIVE = BIND_P<GREATER, ZERO>::F<T>;
template<class T> using NEGATIVE = BIND_P<LESS, ZERO>::F<T>;
template<class T> using POSNULL = BIND_P<GEQ, ZERO>::F<T>;
template<class T> using NEGNULL = BIND_P<LEQ, ZERO>::F<T>;
template<class T> using ISZERO = BIND_P<EQUALS, ZERO>::F<T>;
template<class T> using INC = BIND_P<ADD, ONE>::F<T>;
template<class T> using DEC = BIND_P<ADD, MONE>::F<T>;

namespace test_func {
	static_assert(DEC<int>::f<1>::value == 0, "func");
	static_assert(INC<int>::f<0>::value == 1, "func");
	static_assert(POSITIVE<int>::f<3>::value, "func");
	static_assert(!POSITIVE<int>::f<-2>::value, "func");
	static_assert(NEGATIVE<int>::f<-1>::value, "func");
	static_assert(IDENTITY<char>::f<'a'>::value == 'a', "func");
}

template<class C> struct CONST {
	template<class T> using TF = C;
	template<class T> struct F : C {
		template<T x> using f = C;
	};
	template<class T> struct B : C {
		template<T a, T b> using bin = C;
	};
};



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
template<class T, T... args> struct VALUES : TYPE<T> { using values = VALUES<T, args...>; };
TYPEDEF_TEST(isTypes, types)
TYPEDEF_TEST(isValues, values)
template<class T> constexpr bool isList() { return isValues<T>() || isTypes<T>(); }

namespace test_is {
	static_assert(isValue<TRUE>(), "is");
	static_assert(isTypes<TYPES<void, bool>>(), "is");
	static_assert(isValues<VALUES<int, 1,2,3>>(), "is");
	static_assert(!isList<FALSE>(), "is");
	static_assert(!isValue<NIL>(), "is");
}


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


int main() {
	{
		ints_t<-1, 1, 2, -1, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15> v;
		std::get<3>(v) = 3;
		cout << "sizeof = " << sizeof(v) << endl;
		cout << v << endl;
	}
	cout << endl;
	{
		using T = ints<-1, 1, 2, -1, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15>;
		T v;
		v.write<3>() = 3;
		cout << "sizeof = " << sizeof(v) << " | n = " << v.n << " | n_static = " << v.n_static << " | n_dynamic = " << v.n_dynamic << endl;
		/*cout << T::DYN() << endl;
		cout << T::INDEX() << endl;*/
		cout << v << endl;
	}
	return 0;
}
