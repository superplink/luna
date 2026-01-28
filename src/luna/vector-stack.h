#pragma once
#include "vector.h"



namespace luna {
    

struct SubArray {
    index_t index;
    index_t size;
};



template <class T>
class VectorStackIterator {
public:

    using size_type = index_t;
    using value_type = T;
    using reference = Span<T>;
    using pointer = T*;
    using difference_type = ptrdiff_t;

    constexpr VectorStackIterator () {}

    constexpr VectorStackIterator (T* __data = nullptr, SubArray* __sub_array = nullptr)
    : _data(__data), _sub_array(__sub_array) {}

    constexpr reference operator* () const { return Span<T>(_data, _sub_array->size); }
    // constexpr pointer operator-> () const { return _data; }

    constexpr VectorStackIterator& operator++ () {
        _data += _sub_array->size;
        ++_sub_array;
        return *this;
    }
    constexpr VectorStackIterator operator++ (int) {
        VectorStackIterator a(_data, _sub_array);
        operator++();
        return a;
    }

    constexpr VectorStackIterator& operator-- () {
        --_sub_array;
        _data -= _sub_array->size;
        return *this;
    }
    constexpr VectorStackIterator operator-- (int) {
        VectorStackIterator a(_data, _sub_array);
        operator--();
        return a;
    }

    constexpr bool operator== (const VectorStackIterator& a) const { return _sub_array == a._sub_array; }
    constexpr bool operator!= (const VectorStackIterator& a) const { return _sub_array != a._sub_array; }
    constexpr bool operator<  (const VectorStackIterator& a) const { return _sub_array <  a._sub_array; }
    constexpr bool operator>  (const VectorStackIterator& a) const { return _sub_array >  a._sub_array; }
    constexpr bool operator<= (const VectorStackIterator& a) const { return _sub_array <= a._sub_array; }
    constexpr bool operator>= (const VectorStackIterator& a) const { return _sub_array >= a._sub_array; }

private:

    T* _data;
    SubArray* _sub_array;

};



/**
 * @brief An implimentation of a 2D jagged array where you can only push or pop
 * elements from the top most vector, allowing them to be easily stored in
 * contiguous memory
 */
template <
    ArrayChunk _Chunk,
    ArrayChunkTypeC<SubArray> _SubArrChunk>
class BasicVectorStack {
public:

    using value_type = typename _Chunk::value_type;

    using size_type = index_t;
    using index_type = Index<Span<value_type>>;

    using iterator = VectorStackIterator<value_type>;
    using const_iterator = VectorStackIterator<const value_type>;

    void push_vector () {
        _sub_arrays.push_back({_elts.size(), 0});
    }
    void pop_vector () {
        _elts.resize(_elts.size() - _sub_arrays.back().size);
        _sub_arrays.pop_back();
    }

    void push_back (const value_type& val) {
        _elts.push_back(val);
        _sub_arrays.back().size++;
    }
    template <class... _Args>
    value_type& emplace_back (_Args&&... args) {
        return _elts.emplace_back(std::forward<_Args>(args)...);
    }
    void pop_back () {
        _elts.pop_back();
        _sub_arrays.back().size--;
    }

    Span<value_type> at (index_type index) {
        return Span<value_type>(
            _elts.begin() + _sub_arrays[(index_t)index].index,
            _sub_arrays[(index_t)index].size
        );
    }
    const Span<value_type> at (index_type index) const {
        return Span<value_type>(
            _elts.begin() + _sub_arrays[(index_t)index].index,
            _sub_arrays[(index_t)index].size
        );
    }
    Span<value_type> operator[] (index_type index) { return at(index); }
    const Span<value_type> operator[] (index_type index) const { return at(index); }

    size_type size () const { return _sub_arrays.size(); }

    Span<value_type> front () { return Span<value_type>(_elts.begin(), _sub_arrays.front().size); }
    Span<value_type> back () { return Span<value_type>(_elts.begin() + _sub_arrays.back().index, _sub_arrays.front().size); }
    const Span<value_type> front () const { return Span<value_type>(_elts.begin(), _sub_arrays.front().size); }
    const Span<value_type> back () const { return Span<value_type>(_elts.begin() + _sub_arrays.back().index, _sub_arrays.front().size); }

    iterator begin () { return iterator(_elts.begin(), _sub_arrays.begin()); }
    iterator end () { return iterator(_elts.end(), _sub_arrays.end()); }
    const_iterator begin () const { return const_iterator(_elts.begin(), _sub_arrays.begin()); }
    const_iterator end () const { return const_iterator(_elts.end(), _sub_arrays.end()); }

private:

    BasicVector<_SubArrChunk> _sub_arrays;
    BasicVector<_Chunk> _elts;

};


template <class T>
using VectorStack = BasicVectorStack<HeapArrayChunk<T>, HeapArrayChunk<SubArray>>;





} // namespace luna




