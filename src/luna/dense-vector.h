#pragma once
#include "index.h"
#include "pools.h"
#include <memory>
#include <iterator>
#include "vector.h"
#include "iterator-utils.h"


namespace luna {
    


class RemoveChain {
public:

    using size_type = index_t;
    using value_type = size_type;

    void clear () {
        _chain.clear();
    }

    size_type push () {
        if (is_full()) {
            _chain.push_back(nullindex);
            return _chain.size() - 2;
        }
        size_type index = _root;
        _root = _chain[_root];
        _chain[index] = nullindex;
        _remove_count--;
        return index;
    }

    void remove (size_type index) {
        _chain[index] = _root;
        _root = index;
        _remove_count++;
    }

    size_type size () const { return _chain.size() - _remove_count; }
    size_type full_size () const { return _chain.size(); }

    value_type* begin () { return _chain.begin(); }
    value_type* end () { return _chain.end(); }

    const value_type* begin () const { return _chain.begin(); }
    const value_type* end () const { return _chain.end(); }

    bool is_full () const { return _remove_count == 0; }
    bool is_valid (size_type index) const {
        return _chain[index] == nullindex;
    }

private:

    size_type _remove_count = 0;
    Vector<value_type> _chain;
    size_type _root = tombstone;

};


template <class It>
class RemoveChainValueIterator {
public:

    using size_type = index_t;
    using value_type = typename std::iterator_traits<It>::value_type;
    using reference = typename std::iterator_traits<It>::reference;
    using pointer = typename std::iterator_traits<It>::pointer;

    constexpr RemoveChainValueIterator (It __it, size_type* __chain_ptr, size_type* __chain_end)
    : _it(__it), _chain_ptr(__chain_ptr), _chain_end(__chain_end) {}

    constexpr reference operator* () const { return *_it; }
    constexpr pointer operator-> () const { return _it; }

    constexpr RemoveChainValueIterator& operator++ () {
        do {
            _it++;
            _chain_ptr++;
        } while (*_chain_ptr != nullindex && _chain_ptr != _chain_end);
        return *this;
    }
    constexpr RemoveChainValueIterator operator++ (int) {
        RemoveChainValueIterator a(_it, _chain_ptr, _chain_end);
        operator++();
        return a;
    }

    constexpr RemoveChainValueIterator& operator-- () {
        do {
            _it--;
            _chain_ptr--;
        } while (*_chain_ptr != nullindex && _chain_ptr != _chain_end);
        return *this;
    }
    constexpr RemoveChainValueIterator operator-- (int) {
        RemoveChainValueIterator a(_it, _chain_ptr, _chain_end);
        operator--();
        return a;
    }

    constexpr bool operator== (const RemoveChainValueIterator& a) const { return _it == a._it; }
    constexpr bool operator!= (const RemoveChainValueIterator& a) const { return _it != a._it; }
    constexpr bool operator<  (const RemoveChainValueIterator& a) const { return _it <  a._it; }
    constexpr bool operator>  (const RemoveChainValueIterator& a) const { return _it >  a._it; }
    constexpr bool operator<= (const RemoveChainValueIterator& a) const { return _it <= a._it; }
    constexpr bool operator>= (const RemoveChainValueIterator& a) const { return _it >= a._it; }

private:

    size_type* _chain_ptr;
    size_type* _chain_end;
    It _it;

};


template <class It>
class RemoveChainIPairView {
public:

    using size_type = index_t;
    using iterator = RemoveChainValueIterator<IPairIterator<It>>;

    RemoveChainIPairView (It __begin, It __end, size_type* __chain_ptr, size_type* __chain_end)
    : _begin_it(__begin), _end_it(__end), _chain_ptr(__chain_ptr), _chain_end(__chain_end) {}

    iterator begin () {
        return iterator(make_ipair_iterator(0, _begin_it), _chain_ptr, _chain_end);
    }
    iterator end () {
        return iterator(make_ipair_iterator(nullindex, _end_it), _chain_ptr, _chain_end);
    }

private:

    It _begin_it;
    It _end_it;
    size_type* _chain_ptr;
    size_type* _chain_end;

};


template <class T>
struct RemoveChainUninitializedMove {
    using size_type = index_t;
    
    template <class _InputIt, class _ForwardIt>
    _ForwardIt move (_InputIt first, _InputIt last, _ForwardIt result) const {
        size_type size = last - first; 
        _ForwardIt result_end = result + size;
        assert(last - first == _remove_chain.size());
        for (size_type i = 0; i < size; i++) {
            if (_remove_chain[i] == nullindex) {
                new(result) T(std::move(*first));
            }
            result++;
            first++;
        }
        return result_end;
    }
    
    template <class _InputIt, class _ForwardIt>
    _ForwardIt copy (_InputIt first, _InputIt last, _ForwardIt result) const {
        size_type size = last - first; 
        _ForwardIt result_end = result + size;
        assert(last - first == _remove_chain.size());
        for (size_type i = 0; i < size; i++) {
            if (_remove_chain[i] == nullindex) {
                new(result) T(*first);
            }
            result++;
            first++;
        }
        return result_end;
    }

    Span<size_type> _remove_chain;
};



template <class T, ArrayPool _Pool = HeapArrayPool<T>>
class DenseVector {
public:

    using value_type = T;
    using size_type = index_t;  
    using index_type = Index<T>;
    using pool_type = PushArrayPool<T, _Pool>;

    using iterator = RemoveChainValueIterator<T*>;
    using const_iterator = RemoveChainValueIterator<const T*>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using allocator = pool_type::allocator;

    DenseVector () {}

    template <ArrayPool _OtherPool>
    DenseVector (const DenseVector<T, _OtherPool>& other) {
        clear();
        reserve(other.capacity());
        _pool.push_back(other._pool.size());
        _get_mv().copy(other.begin(), other.end(), _pool.begin());
    }

    template <ArrayPool _OtherPool>
    DenseVector (DenseVector<T, _OtherPool>&& other) {
        clear();
        reserve(other.capacity());
        _pool.push_back(other._pool.size());
        _get_mv().move(other.begin(), other.end(), _pool.begin());
    }

    template <ArrayPool _OtherPool>
    DenseVector& operator= (const DenseVector<T, _OtherPool>& other) {
        clear();
        reserve(other.capacity());
        _pool.push_back(other._pool.size());
        _get_mv().copy(other.begin(), other.end(), _pool.begin());
        return *this;
    }

    template <ArrayPool _OtherPool>
    DenseVector& operator= (DenseVector<T, _OtherPool>&& other) {
        clear();
        reserve(other.capacity());
        _pool.push_back(other._pool.size());
        _get_mv().move(other.begin(), other.end(), _pool.begin());
        return *this;
    }

    ~DenseVector () {
        _destroy_elts();
    }


    template <class... _Args>
    std::pair<index_type, T&> emplace_back (_Args&&... args) {
        if (_removed.is_full()) {
            if (_pool.is_full()) {
                reserve(std::max(_pool.size() * 2, 1));
            }
            T* ptr = _pool.push_back();
            _pool.construct(ptr, std::forward<_Args>(args)...);
            return std::pair<index_type, T&>(_removed.push(), *ptr);
        }
        index_type index = _removed.push();
        _pool.construct(index, std::forward<_Args>(args)...);
        return std::pair<index_type, T&>(index, _pool.at(index));
    }

    index_type push_back (const T& val) {
        return emplace_back(val).first;
    }

    void remove (index_type index) {
        _pool.destroy(index);
        _removed.remove(index);
    }

    void reserve (size_type count) {
        if (is_full())
            _pool.reserve_move(count);
        else
            _pool.reserve_move(count, _get_mv());
    }

    void clear () {
        _destroy_elts();
        _pool.clear();
        _removed.clear();
    }

    size_type size () const { return _removed.size(); }
    size_type full_size () const { return _removed.full_size(); }
    bool is_full () const { return _removed.is_full(); }


    iterator begin () { return iterator(_pool.begin(), _removed.begin(), _removed.end()); }
    iterator end () { return iterator(_pool.end(), _removed.end(), _removed.end()); }
    const_iterator begin () const { return const_iterator(_pool.begin(), _removed.begin(), _removed.end()); }
    const_iterator end () const { return const_iterator(_pool.end(), _removed.end(), _removed.end()); }

    reverse_iterator rbegin () { return std::make_reverse_iterator(end()); }
    reverse_iterator rend () { return std::make_reverse_iterator(begin()); }
    const_reverse_iterator rbegin () const { return std::make_reverse_iterator(end()); }
    const_reverse_iterator rend () const { return std::make_reverse_iterator(begin()); }

    RemoveChainIPairView<T*> ipairs () {
        return RemoveChainIPairView<T*>(_pool.begin(), _pool.end(), _removed.begin(), _removed.end());
    }
    RemoveChainIPairView<const T*> ipairs () const {
        return RemoveChainIPairView<const T*>(_pool.begin(), _pool.end(), _removed.begin(), _removed.end());
    }

private:

    void _destroy_elts () {
        for (size_type i = _pool.size(); i-- > 0;) {
            if (_removed.is_valid(i))
                _pool.destroy(i);
        }
    }

    RemoveChainUninitializedMove<T> _get_mv () {
        return RemoveChainUninitializedMove<T>{ Span<size_type>(_removed.begin(), _removed.end()) };
    }

    pool_type _pool;
    RemoveChain _removed;

};



} // namespace luna




