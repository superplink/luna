#pragma once
#include <ranges>
#include "index.h"
#include "vector.h"
#include "sparse-set.h"


namespace luna {
    



template <
    ArrayChunk _Chunk,
    ArrayChunkTypeC<index_t> _IndexChunk>
class BasicSparseVector {
public:

    using value_type = typename _Chunk::value_type;
    using vector_type = BasicVector<_Chunk>;
    using sparse_set_type = BasicSparseSet<_IndexChunk>;

    using size_type = index_t;
    using index_type = Index<value_type>;

    using iterator = typename vector_type::iterator;
    using const_iterator = typename vector_type::const_iterator;

    index_type push_back (const value_type& val) {
        _elts.push_back(val);
        return _sset.push();
    }

    template <typename... Args>
    index_type emplace_back (Args&&... args) {
        _elts.emplace_back(std::forward<Args>(args)...);
        return _sset.push();
    }

    void remove (index_type index) {
        _elts.remove(_sset.find(index));
        _sset.remove(index);
    }

    value_type& at (index_type index) { return _elts[_sset.find(index)]; }
    const value_type& at (index_type index) const { return _elts[_sset.find(index)]; }

    value_type& operator[] (index_type index) { return _elts[_sset.find(index)]; }
    const value_type& operator[] (index_type index) const { return _elts[_sset.find(index)]; }

    void clear () {
        _elts.clear();
        _sset.clear();
    }

    size_type size () const { return _sset.size(); }
    size_type full_size () const { return _sset.full_size(); }
    bool is_full () const { return size() == full_size(); }

    iterator begin () { return _elts.begin(); }
    iterator end () { return _elts.end(); }
    const_iterator begin () const { return _elts.begin(); }
    const_iterator end () const { return _elts.end(); }

    auto ipairs () {
        return std::views::zip(_sset.template indexes<value_type>(), _elts);
    }
    auto ipairs () const {
        return std::views::zip(_sset.template indexes<value_type>(), _elts);
    }

    index_type next_index () const {
        return _sset.next_index();
    }

private:

    vector_type _elts;
    sparse_set_type _sset;

};


template <class T>
using SparseVector = BasicSparseVector<HeapArrayChunk<T>, HeapArrayChunk<index_t>>;


} // namespace luna



