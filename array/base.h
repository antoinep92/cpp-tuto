// common stuff

using uint = unsigned int;

using YES = void*;
using NO = bool;
namespace test_yesno {
	static_assert(sizeof(YES) != sizeof(NO), "yesno");
}

template<class T> using VOID = void;
template<class A, class B> constexpr bool convertible() { return std::is_convertible<A,B>::value; }


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








template<class... Ts> struct NARGS;
template<> struct NARGS<> : ZERO<uint> {};
template<class H, class... Ts> struct NARGS<H, Ts...> : inc<NARGS<Ts...>> {};
template<class... Ts> constexpr uint nargs() { return NARGS<Ts...>::value; }




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


struct NIL { using nil_tag = void; };
TYPEDEF_TEST(isNil, nil_tag)
