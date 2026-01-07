#pragma once
#include <ranges>
#include "index.h"
#include "vector.h"
#include "sparse-set.h"


namespace luna {
    



template <
    class T,
    ArrayPool _Pool = HeapArrayPool<T>,
    ArrayPool _SparsePool = HeapArrayPool<DenseIndex>,
    ArrayPool _DensePool = HeapArrayPool<SparseIndex>>
class BasicSparseVector {
public:

    using vector_type = BasicVector<T, _Pool>;
    using sparse_set_type = BasicSparseSet<_SparsePool, _DensePool>;

    using value_type = T;
    using size_type = index_t;
    using index_type = Index<T>;

    using iterator = typename vector_type::iterator;
    using const_iterator = typename vector_type::const_iterator;

    index_type push_back (const T& val) {
        _elts.push_back(val);
        return _sset.push();
    }

    template <typename... Args>
    std::pair<index_type, value_type&> emplace_back (Args&&... args) {
        return { _sset.push(), _elts.emplace_back(std::forward<Args>(args)...) };
    }

    void remove (index_type index) {
        std::swap(_elts[_sset.find(index)], _elts.back());
        _elts.pop_back();
        _sset.remove(index);
    }

    T& at (index_type index) { return _elts[_sset.find(index)]; }
    const T& at (index_type index) const { return _elts[_sset.find(index)]; }

    T& operator[] (index_type index) { return _elts[_sset.find(index)]; }
    const T& operator[] (index_type index) const { return _elts[_sset.find(index)]; }

    void clear () {
        _elts.clear();
        _sset.clear();
    }

    size_type size () const { return _sset.size(); }
    size_type full_size () const { return _sset.full_size(); }

    iterator begin () { return _elts.begin(); }
    iterator end () { return _elts.end(); }
    const_iterator begin () const { return _elts.begin(); }
    const_iterator end () const { return _elts.end(); }

    auto ipairs () {
        return std::views::zip(_sset.template indexes<T>(), _elts);
    }

private:

    vector_type _elts;
    sparse_set_type _sset;

};



template <class T, class _Alloc = std::allocator<T>>
using SparseVector = BasicSparseVector<
    T,
    HeapArrayPool<T, _Alloc>,
    HeapArrayPool<DenseIndex>,
    HeapArrayPool<SparseIndex>
>;



} // namespace luna



