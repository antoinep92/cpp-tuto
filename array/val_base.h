#pragma once // basic containers for types and (integral typed) values -- no-dependency file

template<class T> struct type_t { using type = T; };

enum value_kind { sta, dyn, ref };

template<value_kind K> struct value_kind_t {
	static const value_kind kind = K;
	static const bool isStatic = (K == sta);
	static const bool isDynamic = !isStatic;
};
template<class T, value_kind SD = dyn, T V = T()> struct VALUE;

template<class T, T V> struct VALUE<T, sta, V> : type_t<T>, value_kind_t<sta> {
	static const T value = V;
	operator T () const { return value; }
};

template<class T, T V> struct VALUE<T, dyn, V> : type_t<T>, value_kind_t<dyn> {
	T value;
	operator T () const { return value; }
	operator T & () { return value; }
	template<class U> T & operator=(const U & v) { return value = v; }
	VALUE(T v = T()) : value(v) {}
};

template<class T, T V> struct VALUE<T, ref, V> : type_t<T>, value_kind_t<ref> {
	const T & value;
	operator const T & () const { return value; }
};

template<class T, T V> using static_t = VALUE<T, sta, V>;
template<class T> using dynamic_t = VALUE<T, dyn>; 
template<class T> using ref_t = VALUE<T, ref>;

namespace test_value {
	static_assert(static_t<int,3>::value == 3, "value");
}

template<class T> using zero_t = static_t<T, T(0)>;
template<class T> using one_t = static_t<T, T(1)>;
template<class T> using mone_t = static_t<T, T(-1)>;
using true_t = static_t<bool, true>;
using false_t = static_t<bool, false>;
template<bool b> using bool_t = static_t<bool, b>;
template<int i> using int_t = static_t<int, i>;