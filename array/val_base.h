#pragma once // basic containers for types and (integral typed) values -- no-dependency file

template<class T> struct TYPE {
	using type_base = TYPE<T>;
	using type = T;
};

enum value_kind { sta, dyn, ref };

template<value_kind K> struct value_kind_t {
	static const value_kind kind = K;
	static const bool isStatic = (K == sta);
	static const bool isDynamic = !isStatic;
};
template<class T, value_kind SD = dyn, T V = T()> struct value_t;

template<class T, T v_> struct value_t<T, sta, v_> : TYPE<T>, value_kind_t<sta> {
	using value_base = value_t<T, sta, v_>;
	static const T value = v_;
	constexpr operator T () const { return value; }
};

template<class T, T v_> struct value_t<T, dyn, v_> : TYPE<T>, value_kind_t<dyn> {
	using value_base = value_t<T, dyn, v_>;
	T value;
	operator T () const { return value; }
	operator T & () { return value; }
	template<class U> T & operator=(const U & v) { return value = v; }
	value_t(T v = T()) : value(v) {}
};

template<class T, T v_> struct value_t<T, ref, v_> : TYPE<T>, value_kind_t<ref> {
	using value_base = value_t<T, ref, v_>;
	const T & value;
	operator const T & () const { return value; }
};

template<class T, T V> using CONST = value_t<T, sta, V>;
template<class T> using VAR = value_t<T, dyn>; 
template<class T> using REF = value_t<T, ref>;

namespace test_value {
	static_assert(CONST<int,3>::value == 3, "value");
}

template<class T> using zero_t = CONST<T, T(0)>;
template<class T> using one_t = CONST<T, T(1)>;
template<class T> using mone_t = CONST<T, T(-1)>;
using true_t = CONST<bool, true>;
using false_t = CONST<bool, false>;
template<bool b> using bool_t = CONST<bool, b>;
template<int i> using int_t = CONST<int, i>;