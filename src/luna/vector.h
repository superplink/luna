#pragma once
#include "index.h"
#include "memory.h"
#include <memory>
#include <iterator>


namespace luna {
    


template <class T, ArrayChunk _Pool = HeapArrayChunk<T>>
class BasicVector {
public:

    using value_type = T;
    using size_type = index_t;  
    using index_type = Index<T>;
    using pool_type = PushArrayChunk<T, _Pool>;

    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    BasicVector () {}

    template <ArrayChunk _OtherPool>
    BasicVector (const BasicVector<T, _OtherPool>& other) {
        reserve(other.capacity());
        _pool.push_back(other.size());
        std::uninitialized_copy(other.begin(), other.end(), _pool.begin());
    }

    template <ArrayChunk _OtherPool>
    BasicVector (BasicVector<T, _OtherPool>&& other) {
        reserve(other.capacity());
        _pool.push_back(other.size());
        std::uninitialized_move(other.begin(), other.end(), _pool.begin());
        other._pool.deallocate();
    }

    template <ArrayChunk _OtherPool>
    BasicVector& operator= (const BasicVector<T, _OtherPool>& other) {
        clear();
        reserve(other.capacity());
        _pool.push_back(other.size());
        std::uninitialized_copy(other.begin(), other.end(), _pool.begin());
        return *this;
    }

    template <ArrayChunk _OtherPool>
    BasicVector& operator= (BasicVector<T, _OtherPool>&& other) {
        clear();
        reserve(other.capacity());
        _pool.push_back(other.size());
        std::uninitialized_move(other.begin(), other.end(), _pool.begin());
        other._pool.deallocate();
        return *this;
    }

    ~BasicVector () {
        for (auto it = rbegin(); it != rend(); it++) {
            _pool.destroy(&*it);
        }
        _pool.deallocate();
    }

    void swap (BasicVector& other) {
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
            _pool.set_end(_pool.begin() + count);
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

private:

    pool_type _pool;

};


template <class T, class _Alloc = std::allocator<T>>
using Vector = BasicVector<T, HeapArrayChunk<T, _Alloc>>;

template <class T, index_t _Len, class _Alloc = std::allocator<T>>
using InplaceVector = BasicVector<T, InplaceArrayChunk<T, _Len, _Alloc>>;

template <class T, index_t _InplaceLen, class _Alloc = std::allocator<T>>
using CompactVector = BasicVector<T, CompactArrayChunk<T, _InplaceLen, _Alloc>>;



} // namespace luna



