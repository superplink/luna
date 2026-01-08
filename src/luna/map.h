#pragma once
#include "set.h"



namespace luna {
    


template <class _Key, class _Val>
class BasicMapIterator {
public:

    // using size_type = index_t;
    // using index_type = Index<base_value_type>;
    using value_type = std::pair<_Key, _Val>;
    using reference = std::pair<const _Key&, _Val&>;
    using pointer = _Val*;
    using difference_type = std::ptrdiff_t;

    constexpr BasicMapIterator (_Key* __key = nullptr, _Val* __val = nullptr)
    : _key(__key), _val(__val) {}

    constexpr reference operator* () const {
        return reference(*_key, *_val);
    }

    constexpr pointer operator-> () const {
        return _val;
    }

    constexpr BasicMapIterator& operator++ () {
        ++_key;
        ++_val;
        return *this;
    }
    constexpr BasicMapIterator operator++ (int) {
        BasicMapIterator a(_key, _val);
        operator++();
        return a;
    }

    constexpr BasicMapIterator& operator-- () {
        --_key;
        --_val;
        return *this;
    }
    constexpr BasicMapIterator operator-- (int) {
        BasicMapIterator a(_key, _val);
        operator--();
        return a;
    }

    constexpr bool operator== (const BasicMapIterator& a) const { return _val == a._val; }
    constexpr bool operator!= (const BasicMapIterator& a) const { return _val != a._val; }
    constexpr bool operator<  (const BasicMapIterator& a) const { return _val <  a._val; }
    constexpr bool operator>  (const BasicMapIterator& a) const { return _val >  a._val; }
    constexpr bool operator<= (const BasicMapIterator& a) const { return _val <= a._val; }
    constexpr bool operator>= (const BasicMapIterator& a) const { return _val >= a._val; }


private:

    _Key* _key;
    _Val* _val;

};


template <class _Key, class _Val>
using MapIterator = RemoveChainValueIterator<BasicMapIterator<_Key, _Val>>;



template <
    class _Key,
    class _Val,
    class _Hasher = std::hash<_Key>,
    class _Equal = std::equal_to<_Key>,
    ArrayPool _KeyPool = HeapArrayPool<_Key>,
    ArrayPool _ValPool = HeapArrayPool<_Val>,
    ArrayPool _BucketPool = HeapArrayPool<index_t>>
class Map {
public:

    using size_type = index_t;
    using index_type = Index<_Val>;

    using key_type = _Key;
    using value_type = _Val;
    using hasher = _Hasher;
    using key_equal = _Equal;

    using iterator = MapIterator<_Key, _Val>;
    using const_iterator = MapIterator<_Key, const _Val>;

    template <class... _Args>
    std::pair<index_type, bool> emplace (const _Key& key, _Args&&... args) {
        std::pair<size_type, bool> result = _keys.insert(key);
        if (!result.second) return result;
        index_type index = _vals.emplace_back(std::forward<_Args>(args)...).first;
        return std::make_pair(index, true);
    }

    std::pair<index_type, bool> insert (const _Key& key, const _Val& val) {
        return emplace(key, val);
    }

    _Val* find (const _Key& key) {
        size_type index = _keys.find_index(key);
        return index == nullindex ? nullptr : &_vals[index];
    }
    const _Val* find (const _Key& key) const {
        size_type index = _keys.find_index(key);
        return index == nullindex ? nullptr : &_vals[index];
    }

    _Val& at (const _Key& key) {
        size_type index = _keys.find_index(key);
        assert(index != nullindex);
        return _vals[index];
    }
    const _Val& at (const _Key& key) const {
        size_type index = _keys.find_index(key);
        assert(index != nullindex);
        return _vals[index];
    }

    _Val& operator[] (const _Key& key) {
        return _vals[emplace(key).first];
    }
    const _Val& operator[] (const _Key& key) const {
        return at(key);
    }

    _Val& at_index (index_type index) {
        return _vals[index];
    }
    const _Val& at_index (index_type index) const {
        return _vals[index];
    }

    index_type remove (const _Key& key) {
        size_type removed_index = _keys.remove(key);
        if (removed_index != nullindex) {
            _vals.remove(removed_index);
        }
        return removed_index;
    }

    iterator begin () {
        return iterator(
            BasicMapIterator<_Key, _Val>(_keys._elts._pool.begin(), _vals._pool.begin()),
            _vals._removed.begin(),
            _vals._removed.end()
        );
    }
    iterator end () {
        return iterator(
            BasicMapIterator<_Key, _Val>(_keys._elts._pool.end(), _vals._pool.end()),
            _vals._removed.begin(),
            _vals._removed.end()
        );
    }

    const_iterator begin () const {
        return const_iterator(
            BasicMapIterator<_Key, const _Val>(_keys._elts._pool.begin(), _vals._pool.begin()),
            _vals._removed.begin(),
            _vals._removed.end()
        );
    }
    const_iterator end () const {
        return const_iterator(
            BasicMapIterator<_Key, const _Val>(_keys._elts._pool.end(), _vals._pool.end()),
            _vals._removed.begin(),
            _vals._removed.end()
        );
    }

private:

    Set<_Key, _Hasher, _Equal, _KeyPool, _BucketPool> _keys;
    DenseVector<_Val, _ValPool> _vals;

};




} // namespace luna



