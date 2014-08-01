#pragma once // simple unit testing framework
#include "traits.h"
#include <sstream>
#include <exception>

struct SourceContext {
	std::string file;
	int line;
};
#define HERE SourceContext{__FILE__, __LINE__}
std::ostream & operator<<(std::ostream & s, const SourceContext & context) {
	return s << context.file << ':' << context.line;
}

struct E : std::exception {
	const std::string msg;
	E(const std::string & msg) : msg(msg) {}
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
		std::stringstream ss;
		ss << "Exception at " << context << '\n';
		print_lines(ss, args...);
		return ss.str();
	}
	
};

struct EUnitFail : ELocalizedException {
	template<class... Args> EUnitFail(const SourceContext & context, const Args & ... args) : ELocalizedException(context, "Unit test failed", args...) {}
};

template<bool b> struct t_sta;
template<> struct t_sta<true> {};

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
