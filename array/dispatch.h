#pragma once // disptatch runtime evaluated conditions to static metaprogrammed logic
#include "traits.h"
#include "test.h"

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

/// TODO: unit test DISPATCH