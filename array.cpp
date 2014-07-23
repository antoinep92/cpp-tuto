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

namespace meta {

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

template<int Arity, class Func, class... Args> struct FSPEC {
	using R = Func::template R<Args...>;
	static inline constexpr R f(const Args & args...) { return Func::f(args...); }
	template<Args... args> using F = STATIC<R, f(args...)>;
};

template<int Arity, class Func> struct FCRTP {
	static const int arity = Arity;
	using func_t = Func;
	using Func::f;
	using Func::template R;
	template<class... Args> struct specialize = cond_t<count_t<Args...>() == Arity, FSPEC<Arity, Func, Args...>, NIL> {};
};

template<int Arity, int Arg> struct select_fn : FCRTP<Arity, select_fn<Arity, Arg>> {
	template<class... Args> using R = select_t<Arg, Args...>;
	template<class... Args> R<Args...> static inline constexpr f(const Args & args...) { return select<Arg>(args...); }
};
struct cond_f : FCRTP<3, cond_f> {
	template<class C, class T, class F> using R = common<T,F>;
	template<class C, class T, class F> static constexpr inline R<C,T,F> f(const C & c, const T & t, const F & f) { return c ? t : f; }
	//template<class C, C c> struct bind = cond_type<c, select_fn<2,0>, select_fn<2,1>>;
};

template<class Func, int arg_id, class arg_t, arg_t arg> struct bind_f : FCRTP<Func::arity-1, bind_f> {
	
};

enum binop_tag {
	op2_add, op2_sub, op2_mul, op2_div, op2_mod,
	op2_land, op2_lor,
	op2_band, op2_bor, op2_bxor,
	op2_ceq, op2_cneq, op2_clt, op2_cgt, op2_cle, op2_cge
};
enum unop_tag {
	op1_lnot, op1_bnot, op1_minus, op1_plus
};

enum binop_symetry {
	op_symetric, op_asymetric, op_no_symetry
}; /// TODO: model symetry, associativity, distributivity, etc.


template<class F, class V> using valF = STATIC<decltype(F::f(V::value)), F::f(V::value)>;
template<class F, class U, class V> using valBin = STATIC<decltype(F::bin(U::value, V::value)), F::bin(U::value, V::value)>;

template<class A, class B, class func> struct bin_specialization : TYPE<typename func::template R<A,B>> {
	constexpr static inline typename func::template R<A,B> bin(const A a, const B b) { return func::bin(a,b); }
	template<A a, B b> using BIN = STATIC<typename func::template R<A,B>, func::bin(a,b)>;
};
template<class func> struct BIN_CRTP {
	using bin_t = func;
	template<class A, class B = A> using specialize = bin_specialization<A,B, func>;
};

template<binop_tag op> struct BINOP;
template<binop_tag op, class A, class B> constexpr inline typename BINOP<op>::template specialize<A, B>::type binop(const A a, const B b) { return BINOP<op>::bin(a,b); }
#define BINOP_(_TAG_, _OP_, _INIT_) \
	template<> struct BINOP<op2_ ## _TAG_> : STATIC<binop_tag, op2_ ## _TAG_>, BIN_CRTP<BINOP<op2_ ## _TAG_>> { \
		template<class A, class B = A> using R = decltype(declval<A>() _OP_ declval<B>()); \
		template<class A, class B> constexpr static inline R<A,B> bin(const A a, const B b) { return a _OP_ b; } \
		template<class A, class B = A> using init = STATIC<R<A,B>, _INIT_>; \
	}; \
	using _TAG_ = BINOP<op2_ ## _TAG_>; \
	template<class U, class V> using val_ ## _TAG_ = valBin<_TAG_, U, V>;
	
BINOP_(add, +, 0); BINOP_(sub, +, 0);
BINOP_(mul, *, 1); BINOP_(div, /, 1);
BINOP_(mod, %, 1);
BINOP_(land, &&, true); BINOP_(lor, ||, false);
BINOP_(band, &, 1); BINOP_(bor, |, 0); BINOP_(bxor, ^, 0);
BINOP_(ceq, ==, true); BINOP_(cneq, !=, true);
BINOP_(cge, >=, true); BINOP_(cgt, >,  true);
BINOP_(cle, <=, true); BINOP_(clt, <,  true);

namespace test_bin {
	static_assert(BINOP<op2_add>::bin(3,2) == 5, "bin");
	static_assert(BINOP<op2_add>::specialize<int>::bin(3,2) == 5, "bin");
	static_assert(BINOP<op2_add>::specialize<int>::BIN<3,2>::value == 5, "bin");
	static_assert(binop<op2_add>(3,2) == 5, "bin");
	static_assert(binop<op2_mul>(3,2) == 6, "bin");
	static_assert(binop<op2_ceq>('a', 'a'), "bin");
}

template<class T, class func> struct func_specialization : TYPE<typename func::template R<T>> {
	constexpr static inline typename func::template R<T> f(const T v) { return func::f(v); }
	template<T v> using F = STATIC<typename func::template R<T>, func::f(v)>;
};
template<class func> struct FUNC_CRTP {
	using func_t = func;
	template<class T> using specialize = func_specialization<T, func>;
};
template<unop_tag op> struct UNOP;
template<unop_tag op, class T> constexpr inline typename UNOP<op>::template specialize<T>::type unop(const T v) { return UNOP<op>::f(v); }
#define UNOP(_TAG_, _OP_) \
	template<> struct UNOP<op1_ ## _TAG_> : STATIC<unop_tag, op1_ ## _TAG_>, FUNC_CRTP<UNOP<op1_ ## _TAG_>> { \
		template<class T> using R = decltype(_OP_ declval<T>()); \
		template<class T> constexpr static inline R<T> f(const T v) { return (_OP_ v); } \
	}; \
	using _TAG_ = UNOP<op1_ ## _TAG_>; \
	template<class V> using val_ ## _TAG_ = valF<_TAG_, V>;
UNOP(lnot, !); UNOP(bnot, ~);
UNOP(minus, -); UNOP(plus, +);

namespace test_un {
	static_assert(unop<op1_bnot>(false), "un");
	static_assert(UNOP<op1_bnot>::f(false), "un");
	static_assert(UNOP<op1_bnot>::specialize<bool>::f(false), "un");
	static_assert(UNOP<op1_bnot>::specialize<bool>::F<false>::value, "un");
	static_assert(unop<op1_minus>(-5) == 5, "un");
}


template<class Bin, template<class> class Val> struct BIND_RP : FUNC_CRTP<BIND_RP<Bin, Val>> {
	template<class T> using R = decltype(Bin::bin(declval<T>(), Val<T>::value));
	template<class T> constexpr static inline R<T> f(const T v) { return Bin::bin(v, Val<T>::value); }	
};
template<class Bin, template<class> class Val> struct BIND_LP : FUNC_CRTP<BIND_LP<Bin, Val>> {
	template<class T> using R = decltype(Bin::bin(Val<T>::value, declval<T>()));
	template<class T> constexpr static inline R<T> f(const T v) { return Bin::bin(Val<T>::value, v); }
};

template<class Bin, class X, X val> struct BIND_RV : FUNC_CRTP<BIND_RV<Bin, X, val>> {
	template<class T> using R = decltype(Bin::bin(declval<T>(), val));
	template<class T> constexpr static inline R<T> f(const T v) { return Bin::bin(v, val); }
};
template<class Bin, class X, X val> struct BIND_LV : FUNC_CRTP<BIND_LV<Bin, X, val>> {
	template<class T> using R = decltype(Bin::bin(val, declval<T>()));
	template<class T> constexpr static inline R<T> f(const T v) { return Bin::bin(val, v); }
};


namespace test_bind {
	static_assert(BIND_LP<add, ONE>::specialize<int>::F<2>::value == 3, "bind");
	static_assert(BIND_LV<mul, int, 2>::f(3) == 6, "bind");
}

struct identity : FUNC_CRTP<identity> {
	template<class T> using R = T;
	template<class T> constexpr static inline R<T> f(const T v) { return v; }
};

template<class T> struct SIZEOF : STATIC<size_t, sizeof(T)> {};
template<> struct SIZEOF<void> : STATIC<size_t, 0> {};
using positive = BIND_RP<cgt, ZERO>;
using negative = BIND_P<clt, ZERO>;
using posnull = BIND_P<cge, ZERO>;
using negnull = BIND_P<cle, ZERO>;
using isZero = BIND_P<ceq, ZERO>;
using inc = BIND_P<add, ONE>;
using dec = BIND_P<add, MONE>;
template<class C, C cst> struct const_f : FUNC_CRTP<const_f<C, cst>> {
	template<class T> using R = C;
	template<class T> constexpr static inline R<T> f(const T v) { return cst; }
};

namespace test_func {
	static_assert(dec::specialize<int>::F<1>::value == 0, "func");
	static_assert(inc::f(0) == 1, "func");
	static_assert(positive::f(3), "func");
	static_assert(!positive::f(-2), "func");
	static_assert(negative::f(-1), "func");
	static_assert(identity('a') == 'a', "func");
}

namespace test_valop {
	static_assert(val_inc<ZERO<int>>::value == 1, "valop");
	static_assert(val_add<STATIC<int, 3>, STATIC<int, 10>>::value == 13, "valop");
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



/*	Layout
		Auto sufficient layouts
			Affine transform
				ints Multipliers
				ints Offsets
				...
			Norton order
				TBD : bit masks
		
		Transformed layouts
			Bound layout (Layout source, int which_index, int position)
			Sliced layout (Layout source, int which_index, int from, int to, int step = 1)
			Permutated layout (Layout source, ints permutation)
			
		
		Operations
			get depth -> int
			get/set sizes
			get 1D pos : int[depth] -> int
			bind : int,int -> Bound layout
			slice : int,int,int(,int=1) -> Sliced layout
			bind_multi : index -> Layout
			permutate : Permutated layout
			transpose : Permutated layout
		
		/!\ All data and operations must support static and dynamic modes
		+ TODO : layout builder helpers
*/

enum array_rights {
	array_read = 0;		///< read only
	array_write = 1;	///< array_read + change elements' value
	array_resize = 2;	///< array_write + change (non-static parts of) the array size and layout
};

/// top-level class
template<
	class Element	///< contained element type
,	class Layout	///< size and  ND to 1D transform 
,	class Storage	///< fast 1D indexable storage 
,	array_rights	///< read/write/structure-change rights used to disable parts of the API
> struct array {
	/*
		constructors : copy, move, expressions
		view/element/expression operator[](Index)
		expression operator+,-,*...
	 */
};


} // meta namespace

int main() {
	using namespace meta;
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
