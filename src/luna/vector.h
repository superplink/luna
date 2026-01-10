#pragma once
#include "index.h"
#include "memory.h"
#include <memory>
#include <iterator>


namespace luna {
    


template <class T, GenericChunkC<T> _GenericChunk = GenericHeapChunk>
class Vector {
public:

    using value_type = T;
    using size_type = index_t;  
    using index_type = Index<T>;
    using pool_type = PushArrayChunk<T, GenericChunkType<_GenericChunk, T>>;

    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    Vector () {}
    Vector (size_type count, const T& val = {}) {
        resize(count, val);
    }

    template <GenericChunkC<T> _GC>
    Vector (const Vector<T, _GC>& other) {
        reserve(other.capacity());
        _pool.push_back(other.size());
        std::uninitialized_copy(other.begin(), other.end(), _pool.begin());
    }

    template <GenericChunkC<T> _GC>
    Vector (Vector<T, _GC>&& other) {
        reserve(other.capacity());
        _pool.push_back(other.size());
        std::uninitialized_move(other.begin(), other.end(), _pool.begin());
        other._pool.deallocate();
    }

    template <GenericChunkC<T> _GC>
    Vector& operator= (const Vector<T, _GC>& other) {
        reserve(other.capacity());
        _pool.push_back(other.size());
        std::uninitialized_copy(other.begin(), other.end(), _pool.begin());
        return *this;
    }

    template <GenericChunkC<T> _GC>
    Vector& operator= (Vector<T, _GC>&& other) {
        reserve(other.capacity());
        _pool.push_back(other.size());
        std::uninitialized_move(other.begin(), other.end(), _pool.begin());
        other._pool.deallocate();
        return *this;
    }

    ~Vector () {
        for (auto it = rbegin(); it != rend(); it++) {
            _pool.destroy(&*it);
        }
        _pool.deallocate();
    }

    void swap (Vector& other) {
        std::swap(other._pool, _pool);
    }


    template <class... _Args>
    T& emplace_back (_Args&&... args) {
        if (_pool.is_full()) {
            reserve(std::max(size() * 2, 1));
        }
        T* ptr = _pool.push_back();
        _pool.construct(ptr, std::forward<_Args>(args)...);
        return *ptr;
    }

    void push_back (const T& val) {
        emplace_back(val);
    }

    void resize (size_type count, const T& val = {}) {
        if (count < size()) {
            for (size_type i = size(); i-- > count;) {
                _pool.destroy(i);
            }
            _pool.set_size(count);
        } else if (count > size()) {
            reserve(count);
            T* prev_end = _pool.end();
            _pool.set_full();
            std::uninitialized_fill(prev_end, _pool.end(), val);
        }
    }

    void reserve (size_type count) {
        _pool.reserve_move(count);
    }

    void pop_back () {
        _pool.destroy(_pool.pop_back());
    }

    // removeds elements in a fast way, does not preserve order.
    // works by moving last elements into the removed elements, therefore
    // copying the minimal number of elements
    void remove (index_type index, size_type count = 1) {
        _remove_move(index, count, std::min(count, size() - (index + count)));
    }

    // removeds elements such that their order is preserved, at the cost of performance
    void remove_ordered (index_type index, size_type count = 1) {
        _remove_move(index, count, size() - (index + count));
    }

    void clear () {
        for (size_type i = size(); i-- > 0;) {
            _pool.destroy(i);
        }
        _pool.clear();
    }

    T& at (Index<T> index) { return _pool.at(index); }
    const T& at (Index<T> index) const { return _pool.at(index); }
    T& operator[] (Index<T> index) { return _pool.at(index); }
    const T& operator[] (Index<T> index) const { return _pool.at(index); }

    T& front () { return *_pool.begin(); }
    T& back () { return *(_pool.end() - 1); }

    const T& front () const { return *_pool.begin(); }
    const T& back () const { return *(_pool.end() - 1); }

    size_type size () const { return _pool.size(); }
    size_type capacity () const { return _pool.capacity(); }

    iterator begin () { return _pool.begin(); }
    iterator end () { return _pool.end(); }
    const_iterator begin () const { return _pool.begin(); }
    const_iterator end () const { return _pool.end(); }

    reverse_iterator rbegin () { return std::make_reverse_iterator(_pool.end()); }
    reverse_iterator rend () { return std::make_reverse_iterator(_pool.begin()); }
    const_reverse_iterator rbegin () const { return std::make_reverse_iterator(_pool.end()); }
    const_reverse_iterator rend () const { return std::make_reverse_iterator(_pool.begin()); }

    T* data () { return _pool.data(); }
    const T* data () const { return _pool.data(); }

    bool empty () const { return _pool.size() == 0; }

private:

    void _remove_move (index_type index, size_type remove_count, size_type move_count) {
        ASSERT_IN_RANGE((size_type)index + remove_count, 0, size());
        for (index_type i = index; i < index + remove_count; i++) {
            _pool.destroy(index);
        }
        for (size_type i = 0; i < move_count; i++) {
            _pool.construct(index + i, std::move(_pool.at(size() - move_count + i)));
        }
        _pool.pop_back(remove_count);
    }

    pool_type _pool;

};


template <class T, index_t _Len>
using InplaceVector = Vector<T, GenericInplaceChunk<_Len>>;

template <class T, index_t _InplaceLen>
using CompactVector = Vector<T, GenericCompactChunk<_InplaceLen>>;



} // namespace luna



