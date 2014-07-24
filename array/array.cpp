#include <iostream>
using std::cout; using std::cerr; using std::endl;
#include <string>
using std::string;
#include <sstream>
using std::stringstream;
#include <tuple>
using std::tuple;
#include <exception>
using std::exception;
#include <functional>
using std::enable_if; using std::declval;

/*	conventions

		lowercase or CamelCase	first-class object
		UPPERCASE				type which *inderectly* represents a first-class object
		
			
			using X = STATIC<int, 3>
				X represents 3 (X::value). Indirection => uppercase
			
			static const int x = 3; 
				x is directly 3 => No indirection, lowercase
				
			using T = MAKE_CONST<int>
				T represents const int (T::type). Indirection => uppercase
			
			using T = make_cast<int>
				T is an alias to const int. No indirection => lowercase
			
			struct EMPTY {}
				EMPTY is used as a tag, to represent an empty set. The empty set is not even readable, only a convention => uppercase
*/





namespace test_is {
	static_assert(isValue<TRUE>(), "is");
	static_assert(isTypes<TYPES<void, bool>>(), "is");
	static_assert(isValues<VALUES<int, 1,2,3>>(), "is");
	static_assert(!isList<FALSE>(), "is");
	static_assert(!isValue<NIL>(), "is");
}




/*	Layout
		Auto sufficient layouts
			Affine transform
				ints Multipliers
				ints Offsets
				...
			Norton order
				TBD : bit masks
		
		Transformed layouts
			Bound layout (Layout source, int which_index, int position)
			Sliced layout (Layout source, int which_index, int from, int to, int step = 1)
			Permutated layout (Layout source, ints permutation)
			
		
		Operations
			get depth -> int
			get/set sizes
			get 1D pos : int[depth] -> int
			bind : int,int -> Bound layout
			slice : int,int,int(,int=1) -> Sliced layout
			bind_multi : index -> Layout
			permutate : Permutated layout
			transpose : Permutated layout
		
		/!\ All data and operations must support static and dynamic modes
		+ TODO : layout builder helpers
*/

enum array_rights {
	array_read = 0;		///< read only
	array_write = 1;	///< array_read + change elements' value
	array_resize = 2;	///< array_write + change (non-static parts of) the array size and layout
};

/// top-level class
template<
	class Element	///< contained element type
,	class Layout	///< size and  ND to 1D transform 
,	class Storage	///< fast 1D indexable storage 
,	array_rights	///< read/write/structure-change rights used to disable parts of the API
> struct array {
	/*
		constructors : copy, move, expressions
		view/element/expression operator[](Index)
		expression operator+,-,*...
	 */
};


} // meta namespace

int main() {
	using namespace meta;
	{
		ints_t<-1, 1, 2, -1, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15> v;
		std::get<3>(v) = 3;
		cout << "sizeof = " << sizeof(v) << endl;
		cout << v << endl;
	}
	cout << endl;
	{
		using T = ints<-1, 1, 2, -1, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15>;
		T v;
		v.write<3>() = 3;
		cout << "sizeof = " << sizeof(v) << " | n = " << v.n << " | n_static = " << v.n_static << " | n_dynamic = " << v.n_dynamic << endl;
		/*cout << T::DYN() << endl;
		cout << T::INDEX() << endl;*/
		cout << v << endl;
	}
	return 0;
}
