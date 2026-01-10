#pragma once
#include "memory.h"



namespace luna {
    



template <class T, index_t _Len>
class Array {
public:

    using value_type = T;
    using size_type = index_t;  
    using index_type = Index<T>;
    using pool_type = PushArrayChunk<T, _Pool>;

    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_type size () { return _Len; }

    constexpr T& at (index_type index) {
        ASSERT_IN_RANGE(index, 0, size() - 1);
        return _elts[index];
    }
    constexpr const T& at (index_type index) const {
        ASSERT_IN_RANGE(index, 0, size() - 1);
        return _elts[index];
    }
    constexpr T& operator[] (index_type index) { return at(index); }
    constexpr const T& operator[] (index_type index) const { return at(index); }

    constexpr T& front () { return _elts[0]; }
    constexpr T& back () { return _elts[size() - 1]; }
    constexpr const T& front () const { return _elts[0]; }
    constexpr const T& back () const { return _elts[size() - 1]; }

    iterator begin () { return &_elts[0]; }
    iterator end () { return &_elts[size()]; }
    const_iterator begin () const { return &_elts[0]; }
    const_iterator end () const { return &_elts[size()]; }

    reverse_iterator rbegin () { return std::make_reverse_iterator(begin()); }
    reverse_iterator rend () { return std::make_reverse_iterator(end()); }
    const_reverse_iterator rbegin () const { return std::make_reverse_iterator(end()); }
    const_reverse_iterator rend () const { return std::make_reverse_iterator(begin()); }

    T* data () { return _elts; }
    const T* data () const { return _elts; }

    static constexpr bool empty () { return size() == 0; }

private:

    T _elts[size()];

};



} // namespace luna






