#pragma once
#include "index.h"
#include "pools.h"
#include <memory>
#include <iterator>
#include "vector.h"
#include "dense-vector.h"
#include <cassert>


namespace luna {


struct BucketElt {
    using size_type = index_t;

    bool at_end () const { return started && index == nullindex; }

    size_type index = nullindex;
    size_type prev_index = nullindex;
    size_type bucket = nullindex;
    bool started = false;
};


template <ArrayPool _Pool = HeapArrayPool<index_t>>
class BasicBucketVector {
public:

    using size_type = index_t;

    void resize_buckets (size_type count) {
        std::fill(_bucket_next.begin(), _bucket_next.end(), nullindex);
        _bucket_roots.clear();
        _bucket_roots.resize(count, nullindex);
    }

    BucketElt bucket_start (size_type bucket) const {
        return BucketElt{
            .index = nullindex,
            .prev_index = nullindex,
            .bucket = bucket,
            .started = false
        };
    }

    bool get (BucketElt& elt) const {
        if (!elt.started) {
            elt.started = true;
            elt.index = _bucket_roots[elt.bucket];
        } else {
            elt.prev_index = elt.index;
            elt.index = _bucket_next[elt.index];
        }
        return elt.index != nullindex;
    }

    size_type push_back () {
        _bucket_next.push_back(nullindex);
        return _bucket_next.size() - 1;
    }

    void bucket_append (const BucketElt& elt, size_type index) {
        assert(elt.at_end());
        _set_prev_index(elt, index);
    }

    void bucket_remove (const BucketElt& elt) {
        _set_prev_index(elt, _bucket_next[elt.index]);
        _bucket_next[elt.index] = nullindex;
    }

    size_type bucket_count () const { return _bucket_roots.size(); }
    size_type size () const { return _bucket_next.size(); }

private:

    void _set_prev_index (const BucketElt& elt, size_type index) {
        if (elt.prev_index == nullindex) {
            // assert(_bucket_roots[elt.bucket] != nullindex);
            _bucket_roots[elt.bucket] = index;
        } else {
            _bucket_next[elt.prev_index] = index;
        }
    }

    BasicVector<size_type, _Pool> _bucket_roots;
    BasicVector<size_type, _Pool> _bucket_next;

};

using BucketVector = BasicBucketVector<>;



template <
    class T,
    HasherC<T> _Hasher = BasicHasher<T>,
    CompareC<T, T> _Equal = BasicCmp<T>,
    ArrayPool _Pool = HeapArrayPool<T>,
    ArrayPool _BucketPool = HeapArrayPool<index_t>>
class Set {
public:

    using container_type = DenseVector<T, _Pool>;
    using size_type = index_t;
    using index_type = Index<T>;
    using hasher = _Hasher;
    using key_equal = _Equal;

    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

    template <
        class __Key,
        class __Val,
        HasherC<__Key> __Hasher,
        CompareC<__Key, __Key> __Equal,
        ArrayPool __KeyPool,
        ArrayPool __ValPool,
        ArrayPool __BucketPool>
    friend class Map;

    Set (size_type __bucket_count = 101, size_type __max_depth = 4, size_type __resize_scaler = 4)
    : _max_depth(__max_depth), _resize_scaler(__resize_scaler) {
        _buckets.resize_buckets(__bucket_count);
    }

    std::pair<index_type, bool> insert (const T& val, const _Hasher& __hasher = {}, const _Equal& __key_equal = {}) {
        BucketElt bucket_elt = _find_bucket_elt(val, __hasher, __key_equal);
        if (bucket_elt.at_end()) {
            assert(_buckets.size() >= _elts.next_index());
            if (maybe_rehash()) {
                bucket_elt = _find_bucket_elt(val, __hasher, __key_equal);
            }
            if (_buckets.size() == _elts.next_index()) {
                _buckets.push_back();
            }
            index_type index = _elts.push_back(val);
            _buckets.bucket_append(bucket_elt, index);
            return std::make_pair(index, true);
        }
        return std::make_pair((index_type)bucket_elt.index, false);
    }

    template <class _T>
    index_type remove (const _T& val, const _Hasher& __hasher = {}, const _Equal& __key_equal = {}) {
        BucketElt elt = _find_bucket_elt(val, __hasher, __key_equal);
        if (elt.index == nullindex) return nullindex;
        _buckets.bucket_remove(elt);
        _elts.remove(elt.index);
        return elt.index;
    }

    template <class _T>
    T* find (const _T& val, const _Hasher& __hasher = {}, const _Equal& __key_equal = {}) {
        index_type index = find_index(val, __hasher, __key_equal);
        return index == nullindex ? nullptr : at(index);
    }
    template <class _T>
    const T* find (const _T& val, const _Hasher& __hasher = {}, const _Equal& __key_equal = {}) const {
        index_type index = find_index(val, __hasher, __key_equal);
        return index == nullindex ? nullptr : at(index);
    }

    template <class _T>
    index_type find_index (const _T& val, const _Hasher& __hasher = {}, const _Equal& __key_equal = {}) const {
        return _find_bucket_elt(val, __hasher, __key_equal).index;
    }

    void rehash (size_type __bucket_count, const _Hasher& __hasher = {}, const _Equal& __key_equal = {}) {
        _buckets.resize_buckets(__bucket_count);
        for (auto [i, val] : _elts.ipairs()) {
            BucketElt elt = _find_bucket_elt(val, __hasher, __key_equal);
            _buckets.bucket_append(elt, i);
        }
    }

    bool maybe_rehash () {
        if (_elts.size() <= bucket_count() * _max_depth)
            return false;
        rehash(bucket_count() * _resize_scaler);
        return true;
    }

    T& at (index_type index) { return _elts.at(index); }
    const T& at (index_type index) const { return _elts.at(index); }

    iterator begin () { return _elts.begin(); }
    iterator end () { return _elts.end(); }
    const_iterator begin () const { return _elts.begin(); }
    const_iterator end () const { return _elts.end(); }

    size_type size () const { return _elts.size(); }

    auto ipairs () { return _elts.ipairs(); }
    auto ipairs () const { return _elts.ipairs(); }

    size_type bucket_count () const { return _buckets.bucket_count(); }

private:

    template <class _T>
    size_type _get_bucket (const _T& val, const _Hasher& __hasher) const {
        return __hasher.hash(val) % _buckets.bucket_count();
    }

    template <class _T>
    BucketElt _find_bucket_elt (const _T& val, const _Hasher& __hasher, const _Equal& __cmp) const {
        BucketElt bucket_elt = _buckets.bucket_start(_get_bucket(val, __hasher));
        while (_buckets.get(bucket_elt)) {
            if (__cmp.cmp(_elts[bucket_elt.index], val)) {
                return bucket_elt;
            }
        }
        return bucket_elt;
    }

    BasicBucketVector<_BucketPool> _buckets;
    container_type _elts;
    
    size_type _max_depth = 4;
    size_type _resize_scaler = 4;

};





} // namespace luna


