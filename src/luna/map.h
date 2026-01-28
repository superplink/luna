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
    ArrayChunk _KeyChunk,
    ArrayChunk _ValChunk,
    ArrayChunkTypeC<index_t> _IndexChunk,
    HasherC<typename _KeyChunk::value_type> _Hasher = BasicHasher<typename _KeyChunk::value_type>,
    CompareC<typename _KeyChunk::value_type, typename _KeyChunk::value_type> _Equal = BasicCmp<typename _KeyChunk::value_type>>
class BasicMap {
public:

    using key_type = typename _KeyChunk::value_type;
    using value_type = typename _ValChunk::value_type;

    using size_type = index_t;
    using index_type = Index<value_type>;

    using hasher = _Hasher;
    using key_equal = _Equal;

    using iterator = MapIterator<key_type, value_type>;
    using const_iterator = MapIterator<key_type, const value_type>;

    template <class... _Args>
    std::pair<index_type, bool> emplace_ex (const _Hasher& hash, const _Equal& cmp, const key_type& key, _Args&&... args) {
        std::pair<size_type, bool> result = _keys.insert(key, hash, cmp);
        if (!result.second) return result;
        index_type index = _vals.emplace_back(std::forward<_Args>(args)...);
        return std::make_pair(index, true);
    }
    template <class... _Args>
    std::pair<index_type, bool> emplace (const key_type& key, _Args&&... args) {
        return emplace_ex({}, {}, key, std::forward<_Args>(args)...);
    }

    std::pair<index_type, bool> insert (const key_type& key, const value_type& val, const _Hasher& hash = {}, const _Equal& cmp = {}) {
        return emplace_ex(hash, cmp, key, val);
    }

    template <class _T>
    value_type* find (const _T& key, const _Hasher& hash = {}, const _Equal& cmp = {}) {
        size_type index = _keys.find_index(key, hash, cmp);
        return index == nullindex ? nullptr : &_vals[index];
    }
    template <class _T>
    const value_type* find (const _T& key, const _Hasher& hash = {}, const _Equal& cmp = {}) const {
        size_type index = _keys.find_index(key, hash, cmp);
        return index == nullindex ? nullptr : &_vals[index];
    }

    template <class _T>
    value_type& at (const _T& key, const _Hasher& hash = {}, const _Equal& cmp = {}) {
        size_type index = _keys.find_index(key, hash, cmp);
        assert(index != nullindex);
        return _vals[index];
    }
    template <class _T>
    const value_type& at (const _T& key, const _Hasher& hash = {}, const _Equal& cmp = {}) const {
        size_type index = _keys.find_index(key, hash, cmp);
        assert(index != nullindex);
        return _vals[index];
    }

    value_type& operator[] (const key_type& key) {
        return _vals[emplace(key).first];
    }
    const value_type& operator[] (const key_type& key) const {
        return at(key);
    }

    value_type& at_index (index_type index) {
        return _vals[index];
    }
    const value_type& at_index (index_type index) const {
        return _vals[index];
    }

    template <class _T>
    index_type remove (const _T& key, const _Hasher& hash = {}, const _Equal& cmp = {}) {
        size_type removed_index = _keys.remove(key, hash, cmp);
        if (removed_index != nullindex) {
            _vals.remove(removed_index);
        }
        return removed_index;
    }

    iterator begin () {
        return iterator(
            BasicMapIterator<key_type, value_type>(_keys.data(), _vals.data()),
            _vals.remove_chain_data(),
            _vals.remove_chain_data_end()
        );
    }
    iterator end () {
        return iterator(
            BasicMapIterator<key_type, value_type>(_keys.data_end(), _vals.data_end()),
            _vals.remove_chain_data(),
            _vals.remove_chain_data_end()
        );
    }

    const_iterator begin () const {
        return const_iterator(
            BasicMapIterator<key_type, const value_type>(_keys.data(), _vals.data()),
            _vals.remove_chain_data(),
            _vals.remove_chain_data_end()
        );
    }
    const_iterator end () const {
        return const_iterator(
            BasicMapIterator<key_type, const value_type>(_keys.data_end(), _vals.data_end()),
            _vals.remove_chain_data(),
            _vals.remove_chain_data_end()
        );
    }

private:

    BasicSet<_KeyChunk, _IndexChunk, _Hasher, _Equal> _keys;
    BasicDenseVector<_ValChunk> _vals;

};

template <
    class _Key,
    class _Val,
    HasherC<_Key> _Hasher = BasicHasher<_Key>,
    CompareC<_Key, _Key> _Equal = BasicCmp<_Key>>
using Map = BasicMap<
    HeapArrayChunk<_Key>,
    HeapArrayChunk<_Val>,
    HeapArrayChunk<index_t>,
    _Hasher,
    _Equal
>;


} // namespace luna



