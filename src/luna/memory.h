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
    pool.reserve_move(count, UninitializedMove{});

    { pool.size() } -> std::convertible_to<typename T::size_type>;
    { pool.begin() } -> std::convertible_to<typename T::value_type*>;
    { pool.end() } -> std::convertible_to<typename T::value_type*>;
    { pool.data() } -> std::convertible_to<typename T::value_type*>;
    
    { pool.at(index) } -> std::convertible_to<typename T::value_type>;
};


// an inline array that does not call constructors or destructors
template <class T, index_t _Len>
class UninitializedArray {
public:

    using size_type = index_t;

    static constexpr size () { return _Len; }

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
    void reserve_move (size_type count, const _Move& mv = UninitializedMove{}) {
        if (count <= size()) return;
        T* new_first = alloc_traits::allocate(_alloc, count);
        if (_first) {
            mv.move(_first, _last, new_first);
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

    static constexpr size () { return _Len; }

    // does nothing
    void allocate (size_type count) {
        ASSERT_IN_RANGE(count, 0, size() - 1);
    }
    // does nothing
    void deallocate () {}
    // does nothing
    void reserve_move (size_type count) {
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
    void reserve_move (size_type count, const _Move& mv = UninitializedMove{}) {
        if (count <= size()) return;
        if (count <= _InlineSize) return;
        T* new_first = alloc_traits::allocate(_alloc, count);
        mv.move(begin(), end(), new_first);
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



template <class T, ArrayChunk _Pool = HeapArrayChunk<T>>
class PushArrayChunk {
public:

    using pool_type = _Pool;
    using size_type = index_t;
    using allocator = typename _Pool::allocator;

    PushArrayChunk ()
    : _pool()
    , _end(_pool.begin()) {}

    void allocate (size_type count) {
        _pool.allocate(count);
        _end = _pool.begin();
    }
    void deallocate () {
        _pool.deallocate();
        _end = _pool.begin();
    }
    void clear () {
        _pool.clear();
        _end = _pool.begin();
    }

    template <MoveC<T*, T*> _Move = UninitializedMove>
    void reserve_move (size_type count, const _Move& mv = UninitializedMove{}) {
        size_type length = size();
        _pool.reserve_move(count, mv);
        _end = _pool.begin() + length;
    }

    template <class... _Args>
    void construct (Index<T> index, _Args&&... args) {
        _pool.construct(index, std::forward<_Args>(args)...);
    }
    void destroy (Index<T> index) {
        _pool.destroy(index);
    }

    template <class... _Args>
    void construct (T* ptr, _Args&&... args) {
        _pool.construct(ptr, std::forward<_Args>(args)...);
    }
    void destroy (T* ptr) {
        _pool.destroy(ptr);
    }

    // returns pointer to the next element, does not call constructor
    T* push_back (size_type count = 1) {
        T* prev_end = _end;
        _end += count;
        ASSERT_IN_RANGE(_end, _pool.begin(), _pool.end());
        return prev_end;
    }
    // returns pointer to the prev element, does not call destructor
    T* pop_back (size_type count = 1) {
        _end -= count;
        ASSERT_IN_RANGE(_end, _pool.begin(), _pool.end());
        return _end;
    }

    size_type size () const {
        return _end - _pool.begin();
    }

    size_type capacity () const {
        return _pool.size();
    }

    T* begin () { return _pool.begin(); }
    T* end () { return _end; }
    T* data () { return _pool.data(); }
    T* data_end () { return _pool.end(); }

    const T* begin () const { return _pool.begin(); }
    const T* end () const { return _end; }
    const T* data () const { return _pool.data(); }
    const T* data_end () const { return _pool.end(); }

    T& at (Index<T> index) {
        return _pool.at(index);
    }
    const T& at (Index<T> index) const {
        return _pool.at(index);
    }

    bool is_full () const {
        return _end == _pool.end();
    }

    void set_full () {
        _end = _pool.end();
    }

    void set_end (T* __end) {
        ASSERT_IN_RANGE(__end, _pool.begin(), _pool.end());
        _end = __end;
    }

private:

    pool_type _pool;
    T* _end;

};



} // namespace luna



