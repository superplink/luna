#pragma once
#include "index.h"
#include <tuple>



namespace luna {


template <class T, class... _SubT>
struct Component {
	using size_type = index_t;

	Index<T> index;
	std::tuple<Index<_SubT>...> children;
};



class Registry {
public:



private:
};



	
}










