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


template <class T, UnorderedVectorC _Container>
    requires std::same_as<T, typename _Container::value_type>
class BucketVector {
public:

    using container_type = _Container;
    using size_type = index_t;
    using index_type = Index<T>;

    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

    template <class... _Args>
    std::pair<index_type, T&> emplace_back (_Args&&... args) {
        return _elts.emplace_back(std::forward<_Args>(args)...);
    }

    index_type push_back (const T& val) {
        return _elts.push_back(val);
    }

    T& at (index_type index) { return _elts.at(index); }
    const T& at (index_type index) const { return _elts.at(index); }
    T& operator[] (index_type index) { return _elts[index]; }
    const T& operator[] (index_type index) const { return _elts[index]; }

    size_type size () const { return _elts.size(); }

    iterator begin () { return _elts.begin(); }
    iterator end () { return _elts.end(); }
    const_iterator begin () const { return _elts.begin(); }
    const_iterator end () const { return _elts.end(); }

    void remove (index_type index) {
        _elts.remove(index);
    }

    void resize_buckets (size_type count) {
        std::fill(_bucket_next.begin(), _bucket_next.end(), nullindex);
        _bucket_roots.clear();
        _bucket_next.resize(count, nullindex);
    }


    BucketElt bucket_start (size_type bucket) {
        return BucketElt{
            .index = nullindex,
            .prev_index = nullindex,
            .bucket = bucket,
            .started = false
        };
    }

    bool get (BucketElt& elt) {
        if (!elt.started) {
            elt.started = true;
            elt.index = _bucket_roots[elt.bucket];
            return true;
        }
        elt.prev_index = elt.index;
        elt.index = _bucket_next[elt.index];
        return elt.index != nullindex;
    }

    void bucket_append (BucketElt elt, index_type index) {
        assert(elt.at_end());
        _set_prev_index(elt, index);
    }

    void bucket_remove (BucketElt elt) {
        _set_prev_index(elt, _bucket_next[elt.index]);
        _bucket_next[elt.index] = nullindex;
    }

    size_type bucket_count () const { return _bucket_roots.size(); }

private:

    void _set_prev_index (const BucketElt& elt, index_type index) {
        if (elt.prev_index == nullindex) {
            assert(_bucket_roots[elt.bucket] == nullindex);
            _bucket_roots[elt.bucket] = index;
        } else {
            _bucket_next[elt.prev_index] = index;
        }
    }

    container_type _elts;
    Vector<size_type> _bucket_roots;
    Vector<size_type> _bucket_next;

};



template <
    class T,
    class _Hasher = std::hash<T>,
    class _Equal = std::equal_to<T>,
    UnorderedVectorC _Container = DenseVector<T>>
class BasicSet {
public:

    using container_type = _Container;
    using size_type = index_t;
    using index_type = Index<T>;
    using hasher = _Hasher;
    using key_equal = _Equal;

    std::pair<index_type, bool> insert (const T& val) {
        BucketElt bucket_elt = _find_bucket_elt(val);
        if (bucket_elt.at_end()) {
            index_type index = _elts.push_back(val);
            _elts.bucket_append(bucket_elt, index);
            return std::make_pair(index, false);
        }
        return std::make_pair((index_type)bucket_elt.index, true);
    }

    bool remove (const T& val) {
        BucketElt bucket_elt = _find_bucket_elt(val);
        if (bucket_elt.at_end()) return false;
        remove_index(bucket_elt.index);
    }

    T* find (const T& val) {
        index_type index = find_index(val);
        return index == nullindex ? nullptr : at(index);
    }
    const T* find (const T& val) const {
        index_type index = find_index(val);
        return index == nullindex ? nullptr : at(index);
    }

    index_type find_index (const T& val) const {
        return _find_bucket_elt(val).index;
    }

    void remove_index (index_type index) {
        _elts.remove(index);
        _elts.bucket_remove(index);
    }

    T& at (index_type index) { return _elts.at(index); }
    const T& at (index_type index) const { return _elts.at(index); }

private:

    size_type _get_bucket (const T& val) const {
        return hasher{}(val) % _elts.bucket_count();
    }

    BucketElt _find_bucket_elt (const T& val) const {
        BucketElt bucket_elt = _elts.bucket_start(_get_bucket(val));
        while (_elts.get(bucket_elt)) {
            if (key_equal{}(val, _elts[bucket_elt]))
                return bucket_elt;
        }
        return bucket_elt;
    }

    BucketVector<T, _Container> _elts;

};





} // namespace luna


