#pragma once // basic containers for types and (integral typed) values
#include "val_base.h"
#include "test.h"
#include <iostream>


template<class T, ValueKind SD, T V> std::ostream & operator<<(std::ostream & s, const VALUE<T, SD, V> & val) {
	if(SD == sta) s << '$';
	return s << val.value;
}

namespace test_value {
	static t_dyn u2(HERE, DYNAMIC<int>(5).value == 5);
	static t_dyn u3(HERE, []() { DYNAMIC<char> v; v = 'a'; return v == 'a'; });
}
namespace test_value_print {
	t_dyn u1(HERE, []() {
		STATIC<int,5> s;
		DYNAMIC<int> d(3);
		std::stringstream ss;
		ss << s << ' ' << d;
		return ss.str() == "$5 3";
	});
}

TYPEDEF_TEST(type)
FIELD_TEST(value)
template<class T> constexpr bool is_value() { return has_type_type<T>() && has_field_value<T>(); }
