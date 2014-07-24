// function traits, meta functions, etc.

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
