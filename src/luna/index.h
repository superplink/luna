#pragma once
#include <cassert>
#include <iostream>


using index_t = int;
using indexdiff_t = int;

struct nullindex_t {
    constexpr operator int () const { return -1; }
};
static constexpr nullindex_t nullindex = {};

struct tombstone_t {
    constexpr operator int () const { return -2; }
};
static constexpr tombstone_t tombstone = {};


// typesafe indexing
template <class T>
class Index {
public:

    constexpr Index () : value(nullindex) {}
    constexpr Index (index_t v) : value(v) {}
    constexpr Index (nullindex_t) : value(nullindex) {}

    constexpr operator index_t () const { return value; }

    template <class U>
    explicit constexpr operator Index<U> () { return value; }

    constexpr bool operator== (index_t n) const { return value == n; }
    constexpr bool operator!= (index_t n) const { return value != n; }

    constexpr bool operator== (Index<T> n) const { return value == n.value; }
    constexpr bool operator!= (Index<T> n) const { return value != n.value; }

    constexpr bool operator== (nullindex_t) const { return value == -1; }
    constexpr bool operator!= (nullindex_t) const { return value != -1; }

    Index& operator++ () {
        value++;
        return *this;
    }

    Index operator++ (index_t) {
        Index tmp = *this;
        ++*this;
        return tmp;
    }

    Index& operator+= (index_t diff) {
        value += diff;
        return *this;
    }
    Index& operator-= (index_t diff) {
        value += diff;
        return *this;
    }

    Index operator+ (index_t diff) {
        return Index(value + diff);
    }
    Index operator- (index_t diff) {
        return Index(value - diff);
    }

    template <class U>
    friend std::ostream& operator<< (std::ostream& os, Index<U> val);

private:
    index_t value;
};

template <class T>
std::ostream& operator<< (std::ostream& os, Index<T> val) {
    os << val.value;
    return os;
}



namespace luna {
    

template <class T>
concept IterableC = requires (T x) {
    { x.begin() } -> std::same_as<typename T::iterator>;
    { x.end() } -> std::same_as<typename T::iterator>;
};


template <class T>
inline constexpr bool bounds_check (const T& val, const T& low, const T& high) {
    if (val < low || val > high) {
        assert(false);
        return false;
    }
    return true;
}


#ifdef NDEBUG
#define ASSERT_IN_RANGE(__val, __min, __max)
#else
#define ASSERT_IN_RANGE(__val, __min, __max) \
bounds_check(__val, __min, __max)
#endif


// like std::span, but with bounds checking and using luna::Index
template <class T>
class Span {
public:

    using value_type = T;
    using index_type = Index<T>;
    using size_type = int;

    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    Span (T* __begin, size_type length)
    : _begin(__begin), _end(__begin + length) {}

    Span (T* __begin, T* __end)
    : _begin(__begin), _end(__end) {}

    constexpr T& front () { return *_begin; }
    constexpr T& back () { return *(_end - 1); }
    constexpr const T& front () const { return *_begin; }
    constexpr const T& back () const { return *(_end - 1); }

    T& at (index_type index) {
        ASSERT_IN_RANGE(index, 0, size() - 1);
        return _begin[index];
    }
    const T& at (index_type index) const {
        ASSERT_IN_RANGE(index, 0, size() - 1);
        return _begin[index];
    }
    T& operator[] (index_type index) { return at(index); }
    const T& operator[] (index_type index) const { return at(index); }

    iterator begin () { return _begin; }
    iterator end () { return _end; }
    const const_iterator begin () const { return _begin; }
    const const_iterator end () const { return _end; }

    reverse_iterator rbegin () { return std::make_reverse_iterator(begin()); }
    reverse_iterator rend () { return std::make_reverse_iterator(end()); }
    const_reverse_iterator rbegin () const { return std::make_reverse_iterator(end()); }
    const_reverse_iterator rend () const { return std::make_reverse_iterator(begin()); }

    size_type size () const { return _end - _begin; }
    bool empty () const { return _begin == _end; }

    T* data () { return _begin; }
    const T* data () const { return _begin; }

private:

    T* _begin;
    T* _end;

};


template <class _Vec, class... _Args>
concept UnorderedVectorC =
requires (_Vec vec, typename _Vec::index_type index, typename _Vec::value_type val, _Args... args) {
    { vec.emplace_back(args...) } -> std::same_as<typename _Vec::index_type>;
    { vec.push_back(val) } -> std::same_as<typename _Vec::index_type>;
    vec.remove(index);
    { vec.at(index) } -> std::convertible_to<typename _Vec::value_type>;
    { vec[index] } -> std::convertible_to<typename _Vec::value_type>;
    vec.clear();
    { vec.begin() } -> std::convertible_to<typename _Vec::iterator>;
    { vec.end() } -> std::convertible_to<typename _Vec::iterator>;
    { vec.ipairs() } -> IterableC;
} &&
requires (const _Vec& vec, typename _Vec::index_type index, typename _Vec::value_type val, _Args... args) {
    { vec.at(index) } -> std::convertible_to<typename _Vec::value_type>;
    { vec[index] } -> std::convertible_to<typename _Vec::value_type>;
    { vec.size() } -> std::convertible_to<typename _Vec::size_type>;
    { vec.begin() } -> std::convertible_to<typename _Vec::const_iterator>;
    { vec.end() } -> std::convertible_to<typename _Vec::const_iterator>;
    { vec.next_index() } -> std::convertible_to<typename _Vec::size_type>;
    { vec.ipairs() } -> IterableC;
};
    

template <class T>
struct BasicHasher {
    static size_t hash (const T& val) {
        return std::hash<T>{}(val);
    }
};
template <class _Hasher, class T>
concept HasherC = requires (const _Hasher& hasher, const T& val) {
    { hasher.hash(val) } -> std::convertible_to<size_t>;
};

template <class T, class U = T>
struct BasicCmp {
    static bool cmp (const T& a, const U& b) {
        return a == b;
    }
};
template <class _Cmp, class T, class U>
concept CompareC = requires (const _Cmp& cmp, const T& a, const U& b) {
    { cmp.cmp(a, b) } -> std::convertible_to<bool>;
};


} // namespace luna


