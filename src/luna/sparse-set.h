#pragma once
#include "index.h"
#include "memory.h"
#include "vector.h"


namespace luna {
    

struct SparseIndex {
    constexpr SparseIndex (index_t n = nullindex) : index(n) {}
    constexpr SparseIndex (nullindex_t) : index(nullindex) {}
    template <class T>
    constexpr SparseIndex (Index<T> n) : index(n) {}

    constexpr operator index_t () { return index; }
    template <class T>
    constexpr operator Index<T> () { return index; }
    
    index_t index;
};
struct DenseIndex {
    constexpr DenseIndex (index_t n = nullindex) : index(n) {}
    constexpr DenseIndex (nullindex_t) : index(nullindex) {}
    template <class T>
    constexpr DenseIndex (Index<T> n) : index(n) {}

    constexpr operator index_t () { return index; }
    template <class T>
    constexpr operator Index<T> () { return index; }
    
    index_t index;
};


template <GenericChunkC<DenseIndex, SparseIndex> _GenericChunk = GenericHeapChunk>
class BasicSparseSet {
public:

    using sparse_vector_type = Vector<DenseIndex, GenericHeapChunk>;
    using dense_vector_type = Vector<SparseIndex, GenericHeapChunk>;

    using size_type = index_t;


    BasicSparseSet () {}
    ~BasicSparseSet () = default;


    SparseIndex push () {
        size_type prev_len = _len;
        _len++;
        if (prev_len == _dense.size()) {
            _sparse.push_back(prev_len);
            _dense.push_back(prev_len);
            return prev_len;
        }
        _sparse[_dense[prev_len]] = prev_len;
        return _dense[prev_len];
    }
    
    void remove (SparseIndex index) {
        _len--;
        std::swap(_dense[_sparse[index]], _dense[_len]);
        _sparse[_dense[_sparse[index]]] = _sparse[index];
        _sparse[index] = nullindex;
    }

    void reserve (size_type count) {
        _sparse.reserve(count);
        _dense.reserve(count);
    }

    DenseIndex find (SparseIndex index) const {
        return _sparse[index];
    }
    SparseIndex index_of (DenseIndex index) const {
        return _dense[index];
    }

    constexpr size_type size () const { return _len; }
    constexpr size_type full_size () const { return _sparse.size(); }
    bool is_full () const { return size() == full_size(); }

    void clear () {
        _sparse.clear();
        _dense.clear();
        _len = 0;
    }

    Span<const size_type> dense () const {
        return Span<const size_type>((const size_type*)_dense.begin(), (const size_type*)_dense.end());
    }

    template <class T>
    Span<const Index<T>> indexes () const {
        return Span<const Index<T>>((const Index<T>*)_dense.begin(), (const Index<T>*)_dense.end());
    }

    size_type next_index () const {
        return is_full() ? _len : _dense[_len];
    }

private:

    sparse_vector_type _sparse;
    dense_vector_type _dense;
    size_type _len = 0;

};

using SparseSet = BasicSparseSet<>;



} // namespace luna



