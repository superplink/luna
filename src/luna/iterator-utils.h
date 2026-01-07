#pragma once
#include "index.h"
#include <ranges>


namespace luna {
    

template <class T>
concept IterableC = requires (T x) {
    { x.begin() } -> std::same_as<typename T::iterator>;
    { x.end() } -> std::same_as<typename T::iterator>;
};


template <class It>
class IPairIterator {
    using base_value_type = typename std::iterator_traits<It>::value_type;
    using base_ref_type = typename std::iterator_traits<It>::reference;
    using base_ptr_type = typename std::iterator_traits<It>::pointer;
public:

    using size_type = index_t;
    using index_type = Index<base_value_type>;
    using value_type = std::pair<index_type, base_value_type>;
    using reference = std::pair<index_type, base_ref_type>;
    using pointer = base_ptr_type;
    using difference_type = typename std::iterator_traits<It>::difference_type;

    constexpr IPairIterator (index_type __index = nullindex, It __it = nullptr)
    : _index(__index), _it(__it) {}

    constexpr reference operator* () const { return reference(_index, *_it); }
    // constexpr pointer operator-> () const { return pointer(&_index, _it); }

    constexpr IPairIterator& operator++ () {
        ++_it;
        ++_index;
        return *this;
    }
    constexpr IPairIterator operator++ (int) {
        IPairIterator a(_index, _it);
        operator++();
        return a;
    }

    constexpr IPairIterator& operator-- () {
        --_it;
        --_index;
        return *this;
    }
    constexpr IPairIterator operator-- (int) {
        IPairIterator a(_index, _it);
        operator--();
        return a;
    }

    constexpr bool operator== (const IPairIterator& a) const { return _it == a._it; }
    constexpr bool operator!= (const IPairIterator& a) const { return _it != a._it; }
    constexpr bool operator<  (const IPairIterator& a) const { return _it <  a._it; }
    constexpr bool operator>  (const IPairIterator& a) const { return _it >  a._it; }
    constexpr bool operator<= (const IPairIterator& a) const { return _it <= a._it; }
    constexpr bool operator>= (const IPairIterator& a) const { return _it >= a._it; }

private:

    It _it;
    size_type _index;

};


template <class T>
inline IPairIterator<T> make_ipair_iterator (Index<typename std::iterator_traits<T>::value_type> index, T it) {
    return IPairIterator<T>(index, it);
}


template <IterableC T>
inline auto ipairs (T& elts) {
    return std::ranges::subrange(make_ipair_iterator(0, elts.begin()), make_ipair_iterator(0, elts.end()));
}
template <IterableC T>
inline auto ipairs (const T& elts) {
    return std::ranges::subrange(make_ipair_iterator(0, elts.begin()), make_ipair_iterator(0, elts.end()));
}



} // namespace luna




