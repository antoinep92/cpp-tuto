// sequence of static / dynamic ints

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
