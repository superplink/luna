#pragma once
#include "index.h"
#include "memory.h"
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
            return _chain.size() - 1;
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

    size_type next_index () const {
        return _root == tombstone ? size() : _root;
    }

private:

    size_type _remove_count = 0;
    Vector<value_type> _chain;
    size_type _root = tombstone;

};


template <class It>
class RemoveChainValueIterator {
public:

    using size_type = const index_t;
    using value_type = typename std::iterator_traits<It>::value_type;
    using reference = typename std::iterator_traits<It>::reference;
    using pointer = typename std::iterator_traits<It>::pointer;
    using difference_type = typename std::iterator_traits<It>::difference_type;

    constexpr RemoveChainValueIterator () {}

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
RemoveChainValueIterator<It> make_remove_chain_iterator (It it, index_t* __chain_ptr, index_t* __chain_end) {
    return RemoveChainValueIterator<It>(it, __chain_ptr, __chain_end);
}


template <class T>
auto make_remove_chain_view (T& container, index_t* __chain_ptr, index_t* __chain_end) {
    return std::ranges::subrange{
        make_remove_chain_iterator(container.begin(), __chain_ptr, __chain_end),
        make_remove_chain_iterator(container.end(), __chain_ptr, __chain_end)
    };
}
template <class T>
auto make_remove_chain_view (const T& container, index_t* __chain_ptr, index_t* __chain_end) {
    return std::ranges::subrange{
        make_remove_chain_iterator(container.begin(), __chain_ptr, __chain_end),
        make_remove_chain_iterator(container.end(), __chain_ptr, __chain_end)
    };
}



template <class T>
concept RemoveChainIterableC = requires (T x) {
    { x.begin() } -> std::same_as<RemoveChainValueIterator<typename T::value_type>>;
    { x.end() } -> std::same_as<RemoveChainValueIterator<typename T::value_type>>;
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
        for (size_type i = 0; i < size; i++) {
            if (_remove_chain[i] == nullindex) {
                new(result) T(*first);
            }
            result++;
            first++;
        }
        return result_end;
    }
    
    size_type* _remove_chain;
};



template <ArrayChunk _Chunk>
class BasicDenseVector {
public:

    using pool_type = PushArrayChunk<_Chunk>;
    using value_type = typename pool_type::value_type;
    using size_type = index_t;  
    using index_type = Index<value_type>;

    using iterator = RemoveChainValueIterator<value_type*>;
    using const_iterator = RemoveChainValueIterator<const value_type*>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using allocator = pool_type::allocator;

    // template <
    //     class __Key,
    //     class __Val,
    //     HasherC<__Key> __Hasher,
    //     CompareC<__Key, __Key> __Equal,
    //     GenericChunkC<__Key, __Val, index_t> __GenericChunk>
    // friend class Map;

    BasicDenseVector () {}

    template <ArrayChunkTypeC<value_type> _OtherPool>
    BasicDenseVector (const BasicDenseVector<_OtherPool>& other) {
        reserve(other.capacity());
        _pool.push_back(other._pool.size());
        _get_mv().copy(other.begin(), other.end(), _pool.begin());
    }

    template <ArrayChunkTypeC<value_type> _OtherPool>
    BasicDenseVector (BasicDenseVector<_OtherPool>&& other) {
        reserve(other.capacity());
        _pool.push_back(other._pool.size());
        _get_mv().move(other.begin(), other.end(), _pool.begin());
    }

    template <ArrayChunkTypeC<value_type> _OtherPool>
    BasicDenseVector& operator= (const BasicDenseVector<_OtherPool>& other) {
        reserve(other.capacity());
        _pool.push_back(other._pool.size());
        _get_mv().copy(other.begin(), other.end(), _pool.begin());
        return *this;
    }

    template <ArrayChunkTypeC<value_type> _OtherPool>
    BasicDenseVector& operator= (BasicDenseVector<_OtherPool>&& other) {
        reserve(other.capacity());
        _pool.push_back(other._pool.size());
        _get_mv().move(other.begin(), other.end(), _pool.begin());
        return *this;
    }

    ~BasicDenseVector () {
        _destroy_elts();
    }


    template <class... _Args>
    index_type emplace_back (_Args&&... args) {
        index_type index = _removed.push();
        if (index == _removed.size() - 1) {
            if (_pool.is_full()) {
                _pool.reserve_move(std::max(_pool.size() * 2, 1));
            }
            _pool.push_back();
        }
        _pool.construct(index, std::forward<_Args>(args)...);
        return index;
    }

    index_type push_back (const value_type& val) {
        return emplace_back(val);
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

    value_type& at (index_type index) { return _pool.at(index); }
    const value_type& at (index_type index) const { return _pool.at(index); }

    value_type& operator[] (index_type index) { return _pool.at(index); }
    const value_type& operator[] (index_type index) const { return _pool.at(index); }

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

    RemoveChainIPairView<value_type*> ipairs () {
        return RemoveChainIPairView<value_type*>(_pool.begin(), _pool.end(), _removed.begin(), _removed.end());
    }
    RemoveChainIPairView<const value_type*> ipairs () const {
        return RemoveChainIPairView<const value_type*>(_pool.begin(), _pool.end(), _removed.begin(), _removed.end());
    }

    index_type next_index () const {
        return _removed.next_index();
    }

    value_type* data () { return _pool.begin(); }
    value_type* data_end () { return _pool.end(); }
    const value_type* data () const { return _pool.begin(); }
    const value_type* data_end () const { return _pool.end(); }

    const size_type* remove_chain_data () const { return _removed.begin(); }
    const size_type* remove_chain_data_end () const { return _removed.end(); }

private:

    void _destroy_elts () {
        for (size_type i = _pool.size(); i-- > 0;) {
            if (_removed.is_valid(i))
                _pool.destroy(i);
        }
    }

    RemoveChainUninitializedMove<value_type> _get_mv () {
        return RemoveChainUninitializedMove<value_type>{ _removed.begin() };
    }

    pool_type _pool;
    RemoveChain _removed;

};


template <class T, class _Alloc = std::allocator<T>>
using DenseVector = BasicDenseVector<HeapArrayChunk<T, _Alloc>>;


static_assert(UnorderedVectorC<DenseVector<int>>);



} // namespace luna




