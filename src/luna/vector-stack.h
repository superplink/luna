#pragma once
#include "vector.h"



namespace luna {
    

struct SubArray {
    index_t index;
    index_t size;
};


/**
 * @brief An implimentation of a 2D jagged array where you can only push or pop
 * elements from the top most vector, allowing them to be easily stored in
 * contiguous memory
 * 
 * @tparam T 
 * @tparam _Chunk 
 */
template <
    class T,
    ArrayChunk _Chunk = HeapArrayChunk<T>,
    ArrayChunk _SizeChunk = HeapArrayChunk<SubArray>>
class VectorStack {
public:

    using size_type = index_t;
    using index_type = Index<Span<T>>;

    void push_vector () {
        _sizes.push_back({_elts.size(), 0});
    }
    void pop_vector () {
        _elts.resize(_elts.size() - _sizes.back());
        _sizes.pop_back();
    }

    void push_back (const T& val) {
        _elts.push_back(val);
        _sub_arrays.back().size++;
    }
    template <class... _Args>
    T& emplace_back (_Args&&... args) {
        return _elts.emplace_back(std::forward<_Args>(args)...);
    }
    void pop_back () {
        _elts.pop_back();
        _sub_arrays.back().size--;
    }

    Span<T> at (index_type index) {
        return Span<T>(
            _elts.begin() + _sub_arrays[(index_t)index].index,
            _sub_arrays[(index_t)index].size
        );
    }
    const Span<T> at (index_type index) const {
        return Span<T>(
            _elts.begin() + _sub_arrays[(index_t)index].index,
            _sub_arrays[(index_t)index].size
        );
    }
    Span<T> operator[] (index_type index) { return at(index); }
    const Span<T> operator[] (index_type index) const { return at(index); }

private:

    BasicVector<SubArray, _SizeChunk> _sub_arrays;
    BasicVector<T, _Chunk> _elts;

};





} // namespace luna




