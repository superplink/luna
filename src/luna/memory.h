#pragma once
#include <memory>
#include "index.h"
#include <cassert>


/**
 * @brief In Luna, pools and containers are abstractions over allocators.
 * An allocator simply manages how memory is allocated.
 * A pool manages how multiple elements are stored.
 * A container provides an interface for a pool, controlling when
 * the constructor or destructor of objects stored in it are called.
 */

namespace luna {
    


struct UninitializedMove {
    template <class _InputIt, class _ForwardIt>
    static _ForwardIt move (_InputIt __first, _InputIt __last, _ForwardIt __result) {
        return std::uninitialized_move(__first, __last, __result);
    }
    template <class _InputIt, class _ForwardIt>
    static _ForwardIt copy (_InputIt __first, _InputIt __last, _ForwardIt __result) {
        return std::uninitialized_copy(__first, __last, __result);
    }
};


template <class _Move, class _InputIt, class _ForwardIt>
concept MoveC = requires (_Move mv, _InputIt __first, _InputIt __last, _ForwardIt __result) {
    mv.move(__first, __last, __result);
    mv.copy(__first, __last, __result);
};


template <class T, class... _Args>
concept ArrayChunk = requires (T pool, typename T::size_type count, typename T::index_type index, _Args... args) {
    pool.allocate(count);
    pool.deallocate();
    pool.construct(index, args...);
    pool.destroy(index);
    pool.clear();
    pool.reserve_move(count, count, UninitializedMove{});

    { pool.size() } -> std::convertible_to<typename T::size_type>;
    { pool.begin() } -> std::convertible_to<typename T::value_type*>;
    { pool.end() } -> std::convertible_to<typename T::value_type*>;
    { pool.data() } -> std::convertible_to<typename T::value_type*>;
    
    { pool.at(index) } -> std::convertible_to<typename T::value_type>;
};

template <class _Chunk, class T>
concept ArrayChunkTypeC = ArrayChunk<_Chunk>
    && std::same_as<typename _Chunk::value_type, T>;


template <class T, class U>
concept MatchingChunkC = ArrayChunk<T>
    && ArrayChunk<U>
    && std::same_as<typename T::value_type, typename U::value_type>;


// an inline array that does not call constructors or destructors
template <class T, index_t _Len>
class UninitializedArray {
public:

    using size_type = index_t;

    static constexpr index_t size () { return _Len; }

    constexpr T* data () { return (T*)_bytes; }
    constexpr const T* data () const { return (T*)_bytes; }

    constexpr T* begin () { return data(); }
    constexpr T* end () { return data() + size(); }

    constexpr const T* begin () const { return data(); }
    constexpr const T* end () const { return data() + size(); }

    constexpr T& operator[] (size_type index) { return data()[index]; }
    constexpr const T& operator[] (size_type index) const { return data()[index]; }

private:

    int8_t _bytes[sizeof(T) * _Len];

};



template <class T, class _Alloc = std::allocator<T>>
class HeapArrayChunk {
public:

    using value_type = T;
    using index_type = Index<T>;
    using size_type = index_t;  
    using allocator = _Alloc;
    using alloc_traits = std::allocator_traits<_Alloc>;

    HeapArrayChunk ()
    : _first(nullptr)
    , _last(nullptr) {}

    void allocate (size_type count) {
        deallocate();
        _first = alloc_traits::allocate(_alloc, count);
        _last = _first + count;
    }

    void deallocate () {
        if (!_first) return;
        alloc_traits::deallocate(_alloc, _first, _last - _first);
        _first = nullptr;
        _last = nullptr;
    }

    template <MoveC<T*, T*> _Move>
    void reserve_move (size_type prev_count, size_type count, const _Move& mv = UninitializedMove{}) {
        if (count <= size()) return;
        T* new_first = alloc_traits::allocate(_alloc, count);
        if (_first) {
            mv.move(_first, _first + prev_count, new_first);
            alloc_traits::deallocate(_alloc, _first, size());
        }
        _first = new_first;
        _last = _first + count;
    }

    template <class... _Args>
    void construct (Index<T> index, _Args&&... args) {
        ASSERT_IN_RANGE((int)index, 0, size() - 1);
        alloc_traits::construct(_alloc, &_first[index], std::forward<_Args>(args)...);
    }
    void destroy (Index<T> index) {
        ASSERT_IN_RANGE((int)index, 0, size() - 1);
        alloc_traits::destroy(_alloc, &_first[index]);
    }

    template <class... _Args>
    void construct (T* ptr, _Args&&... args) {
        alloc_traits::construct(_alloc, ptr, std::forward<_Args>(args)...);
    }
    void destroy (T* ptr) {
        alloc_traits::destroy(_alloc, ptr);
    }

    size_type size () const { return _last - _first; };

    T* begin () const { return _first; }
    T* end () const { return _last; }
    T* data () const { return _first; }

    T& at (Index<T> index) {
        ASSERT_IN_RANGE((int)index, 0, size() - 1);
        return _first[index];
    }
    const T& at (Index<T> index) const {
        ASSERT_IN_RANGE((int)index, 0, size() - 1);
        return _first[index];
    }

    // does nothing
    void clear () {}

private:

    [[no_unique_address]] _Alloc _alloc;
    T* _first;
    T* _last;

};



template <class T, index_t _Len, class _Alloc = std::allocator<T>>
class InplaceArrayChunk {
public:

    using value_type = T;
    using size_type = index_t;  
    using index_type = Index<T>;
    using allocator = _Alloc;
    using alloc_traits = std::allocator_traits<_Alloc>;

    static constexpr index_t size () { return _Len; }

    // does nothing
    void allocate (size_type count) {
        ASSERT_IN_RANGE(count, 0, size() - 1);
    }
    // does nothing
    void deallocate () {}
    // does nothing
    void reserve_move (size_type prev_count, size_type count) {
        ASSERT_IN_RANGE(count, 0, size() - 1);
    }

    template <class... _Args>
    void construct (Index<T> index, _Args&&... args) {
        ASSERT_IN_RANGE((int)index, 0, size() - 1);
        alloc_traits::construct(_alloc, &_elts[index], std::forward<_Args>(args)...);
    }
    void destroy (Index<T> index) {
        ASSERT_IN_RANGE((int)index, 0, size() - 1);
        alloc_traits::destroy(_alloc, &_elts[index]);
    }

    template <class... _Args>
    void construct (T* ptr, _Args&&... args) {
        alloc_traits::construct(_alloc, ptr, std::forward<_Args>(args)...);
    }
    void destroy (T* ptr) {
        alloc_traits::destroy(_alloc, ptr);
    }

    T* begin () { return &_elts[0]; }
    T* end () { return &_elts[size()]; }
    T* data () { return _elts.data(); }

    const T* begin () const { return &_elts[0]; }
    const T* end () const { return &_elts[size()]; }
    const T* data () const { return _elts.data(); }

    T& at (Index<T> index) {
        ASSERT_IN_RANGE((int)index, 0, size() - 1);
        return _elts[index];
    }
    const T& at (Index<T> index) const {
        ASSERT_IN_RANGE((int)index, 0, size() - 1);
        return _elts[index];
    }

    // does nothing
    void clear () {}

private:

    [[no_unique_address]] _Alloc _alloc;
    UninitializedArray<T, _Len> _elts;

};


template <class T, index_t _InlineSize, class _Alloc = std::allocator<T>>
class CompactArrayChunk {
public:

    using value_type = T;
    using size_type = index_t;  
    using index_type = Index<T>;
    using allocator = _Alloc;
    using alloc_traits = std::allocator_traits<_Alloc>;

    size_type size () const { return _size; }

    bool is_compact () const {
        return _size <= _InlineSize;
    }

    void allocate (size_type count) {
        deallocate();
        _size = count;
        if (is_compact()) return;
        _vec = alloc_traits::allocate(_alloc, count);
    }
    void deallocate () {
        if (is_compact()) return;
        alloc_traits::deallocate(_alloc, _vec, _size);
        _size = _InlineSize;
    }

    template <MoveC<T*, T*> _Move>
    void reserve_move (size_type prev_count, size_type count, const _Move& mv = UninitializedMove{}) {
        if (count <= size()) return;
        if (count <= _InlineSize) return;
        T* new_first = alloc_traits::allocate(_alloc, count);
        mv.move(begin(), begin() + prev_count, new_first);
        if (!is_compact()) {
            alloc_traits::deallocate(_alloc, _vec, _size);
        }
        _size = count;
        _vec = new_first;
    }

    template <class... _Args>
    void construct (Index<T> index, _Args&&... args) {
        ASSERT_IN_RANGE((int)index, 0, size() - 1);
        alloc_traits::construct(_alloc, &data()[index], std::forward<_Args>(args)...);
    }
    void destroy (Index<T> index) {
        ASSERT_IN_RANGE((int)index, 0, size() - 1);
        alloc_traits::destroy(_alloc, &data()[index]);
    }

    template <class... _Args>
    void construct (T* ptr, _Args&&... args) {
        alloc_traits::construct(_alloc, ptr, std::forward<_Args>(args)...);
    }
    void destroy (T* ptr) {
        alloc_traits::destroy(_alloc, ptr);
    }

    T* begin () { return data(); }
    T* end () { return data() + size(); }

    const T* begin () const { return data(); }
    const T* end () const { return data() + size(); }

    T& at (Index<T> index) {
        ASSERT_IN_RANGE((int)index, 0, size() - 1);
        return data()[index];
    }
    const T& at (Index<T> index) const {
        ASSERT_IN_RANGE((int)index, 0, size() - 1);
        return data()[index];
    }

    T* data () { return is_compact() ? _arr.data() : _vec; }
    const T* data () const { return is_compact() ? _arr.data() : _vec; }

    // does the same as deallocate
    void clear () {
        deallocate();
    }

private:

    [[no_unique_address]] _Alloc _alloc;

    int _size = _InlineSize;
    union {
        T* _vec;
        UninitializedArray<T, _InlineSize> _arr;
    };

};



template <ArrayChunk _Chunk>
class PushArrayChunk {
public:

    using chunk_type = _Chunk;
    using value_type = typename chunk_type::value_type;
    using size_type = index_t;
    using allocator = typename chunk_type::allocator;
    using index_type = Index<value_type>;

    PushArrayChunk ()
    : _pool()
    , _size(0) {}

    void allocate (size_type count) {
        _pool.allocate(count);
    }
    void deallocate () {
        _pool.deallocate();
    }
    void clear () {
        _pool.clear();
        _size = 0;
    }

    template <MoveC<value_type*, value_type*> _Move = UninitializedMove>
    void reserve_move (size_type count, const _Move& mv = UninitializedMove{}) {
        size_type length = size();
        _pool.reserve_move(length, count, mv);
    }

    template <class... _Args>
    void construct (Index<value_type> index, _Args&&... args) {
        _pool.construct(index, std::forward<_Args>(args)...);
    }
    void destroy (Index<value_type> index) {
        _pool.destroy(index);
    }

    template <class... _Args>
    void construct (value_type* ptr, _Args&&... args) {
        _pool.construct(ptr, std::forward<_Args>(args)...);
    }
    void destroy (value_type* ptr) {
        _pool.destroy(ptr);
    }

    // returns pointer to the next element, does not call constructor
    value_type* push_back (size_type count = 1) {
        value_type* prev_end = end();
        _size += count;
        ASSERT_IN_RANGE(_size, 0, _pool.size());
        return prev_end;
    }
    // returns pointer to the prev element, does not call destructor
    value_type* pop_back (size_type count = 1) {
        _size -= count;
        ASSERT_IN_RANGE(_size, 0, _pool.size());
        return end();
    }

    size_type size () const {
        return _size;
    }

    size_type capacity () const {
        return _pool.size();
    }

    value_type* begin () { return _pool.begin(); }
    value_type* end () { return _pool.begin() + _size; }
    value_type* data () { return _pool.data(); }
    value_type* data_end () { return _pool.end(); }

    const value_type* begin () const { return _pool.begin(); }
    const value_type* end () const { return _pool.begin() + _size; }
    const value_type* data () const { return _pool.data(); }
    const value_type* data_end () const { return _pool.end(); }

    value_type& at (index_type index) {
        return _pool.at(index);
    }
    const value_type& at (index_type index) const {
        return _pool.at(index);
    }

    bool is_full () const {
        return _size == _pool.size();
    }

    void set_full () {
        _size = _pool.size();
    }

    void set_size (size_type __size) {
        _size = __size;
    }

    // void set_end (T* __end) {
    //     ASSERT_IN_RANGE(__end, _pool.begin(), _pool.end());
    //     _end = __end;
    // }

private:

    chunk_type _pool;
    size_type _size;

};



} // namespace luna



