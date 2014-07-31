#pragma once // basic containers for types and (integral typed) values
#include "val_base.h"
#include "test.h"
#include <iostream>



template<class T, value_kind SD, T V> std::ostream & operator<<(std::ostream & s, const VALUE<T, SD, V> & val) {
	if(SD == sta) s << '$';
	return s << val.value;
}

namespace test_value {
	static t_dyn u2(HERE, dynamic_t<int>(5).value == 5);
	static t_dyn u3(HERE, []() { dynamic_t<char> v; v = 'a'; return v == 'a'; });
}
namespace test_value_print {
	t_dyn u1(HERE, []() {
		static_t<int,5> s;
		dynamic_t<int> d(3);
		std::stringstream ss;
		ss << s << ' ' << d;
		return ss.str() == "$5 3";
	});
}

