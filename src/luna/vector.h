#pragma once
#include "index.h"
#include "memory.h"
#include <memory>
#include <iterator>


namespace luna {
    


template <ArrayChunk _Chunk>
class BasicVector {
public:

    using chunk_type = PushArrayChunk<_Chunk>;
    using value_type = typename chunk_type::value_type;
    using size_type = index_t;
    using index_type = Index<value_type>;

    using iterator = value_type*;
    using const_iterator = const value_type*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    BasicVector () {}
    BasicVector (size_type count, const value_type& val = {}) {
        resize(count, val);
    }

    template <ArrayChunkTypeC<value_type> _OtherChunk>
    BasicVector (const BasicVector<_OtherChunk>& other) {
        reserve(other.capacity());
        _pool.push_back(other.size());
        std::uninitialized_copy(other.begin(), other.end(), _pool.begin());
    }

    template <ArrayChunkTypeC<value_type> _OtherChunk>
    BasicVector (BasicVector<_OtherChunk>&& other) {
        reserve(other.capacity());
        _pool.push_back(other.size());
        std::uninitialized_move(other.begin(), other.end(), _pool.begin());
        other._pool.deallocate();
    }

    template <ArrayChunkTypeC<value_type> _OtherChunk>
    BasicVector& operator= (const BasicVector<_OtherChunk>& other) {
        reserve(other.capacity());
        _pool.push_back(other.size());
        std::uninitialized_copy(other.begin(), other.end(), _pool.begin());
        return *this;
    }

    template <ArrayChunkTypeC<value_type> _OtherChunk>
    BasicVector& operator= (BasicVector<_OtherChunk>&& other) {
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
    value_type& emplace_back (_Args&&... args) {
        if (_pool.is_full()) {
            reserve(std::max(size() * 2, 1));
        }
        value_type* ptr = _pool.push_back();
        _pool.construct(ptr, std::forward<_Args>(args)...);
        return *ptr;
    }

    void push_back (const value_type& val) {
        emplace_back(val);
    }
    
    template <class... _Args>
    value_type& emplace (size_type index, _Args&&... args) {
        if (_pool.is_full()) {
            reserve(std::max(size() * 2, 1));
        }
        _pool.push_back();
        for (size_type i = size() - 1; i-- > index;) {
            _pool.construct(i + 1, std::move(_pool.at(i)));
        }
        _pool.construct(index, std::forward<_Args>(args)...);
        return _pool.at(index);
    }
    void insert (size_type index, const value_type& val) {
        emplace(index, val);
    }

    void resize (size_type count, const value_type& val = {}) {
        if (count < size()) {
            for (size_type i = size(); i-- > count;) {
                _pool.destroy(i);
            }
            _pool.set_size(count);
        } else if (count > size()) {
            reserve(count);
            value_type* prev_end = _pool.end();
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

    value_type& at (index_type index) { return _pool.at(index); }
    const value_type& at (index_type index) const { return _pool.at(index); }
    value_type& operator[] (index_type index) { return _pool.at(index); }
    const value_type& operator[] (index_type index) const { return _pool.at(index); }

    value_type& front () { return *_pool.begin(); }
    value_type& back () { return *(_pool.end() - 1); }

    const value_type& front () const { return *_pool.begin(); }
    const value_type& back () const { return *(_pool.end() - 1); }

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

    value_type* data () { return _pool.data(); }
    const value_type* data () const { return _pool.data(); }

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

    chunk_type _pool;

};

template <class T, class _Alloc = std::allocator<T>>
using Vector = BasicVector<HeapArrayChunk<T, _Alloc>>;

template <class T, index_t _Len>
using InplaceVector = BasicVector<InplaceArrayChunk<T, _Len>>;

template <class T, index_t _InplaceLen>
using CompactVector = BasicVector<CompactArrayChunk<T, _InplaceLen>>;



} // namespace luna



