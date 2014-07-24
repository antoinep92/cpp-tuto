#pragma once // basic containers for types and (integral typed) values -- no-dependency file

template<class T> struct TYPE { using type = T; };

enum ValueKind { sta, dyn, ref };

template<ValueKind K> struct VALUE_KIND {
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

template<class T> using ZERO = STATIC<T, T(0)>;
template<class T> using ONE = STATIC<T, T(1)>;
template<class T> using MONE = STATIC<T, T(-1)>;
using TRUE = STATIC<bool, true>;
using FALSE = STATIC<bool, false>;

template<class T> struct MAKE_VALUE {
	template<T x> using f = STATIC<T,x>;
};
