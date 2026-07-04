#pragma once
#include "index.h"
#include <algorithm>
#include <cstring>
#include <string>
#include <string_view>


namespace luna {


template <index_t _Sz, class T = char>
class BufferString {
public:

    using size_type = index_t;
    using index_type = Index<T>;

    BufferString () {}
    BufferString (const char* str) {
        char* last = std::strncpy(_str, str, buffer_size());
        _len = last - _str;
    }
    template <index_t _OLen, class OT>
    BufferString (const BufferString& other) {
        std::strncpy(_str, other._str, buffer_size());
        _len = other._len;
    }
    BufferString (std::string_view str) {
        std::strncpy(_str, str.data(), std::min((size_type)str.size(), buffer_size()));
        _len = str.length();
    }
    BufferString (const std::string& str) {
        std::strncpy(_str, str.data(), buffer_size());
        _len = str.length();
    }

    static constexpr size_type capacity () { return _Sz; }
    static constexpr size_type buffer_size () { return _Sz; }
    size_type length () const {
        return _len;
    }
    
    bool operator== (const char* str) const {
        return std::strncmp(_str, str, buffer_size()) == 0;
    }

    template <index_t _OLen, class OT>
    bool operator== (const BufferString<_OLen, OT>& other) const {
        return std::strncmp(_str, other._str, buffer_size()) == 0;
    }

    void clear () {
        std::fill(_str, _str + buffer_size(), 0);
        _len = 0;
    }

    std::string to_string () const {
        return _str;
    }

    char* data () { return _str; }
    const char* data () const { return _str; }

    const char* c_str () const { return _str; }

    char* begin () { return _str; }
    char* end () { return _str + _len; }
    const char* cbegin () const { return _str; }
    const char* cend () const { return _str + _len; }
    const char* begin () const { return _str; }
    const char* end () const { return _str + _len; }

    char& operator[] (size_type index) { return _str[index]; }
    char operator[] (size_type index) const { return _str[index]; }
    
private:

    T _str[_Sz] = {};
    size_type _len = 0;
    
};



} // namespace luna

namespace std {
template <index_t _Len, class T>
struct hash<luna::BufferString<_Len, T>> {
    size_t operator ()(const luna::BufferString<_Len, T>& str) const {
        return std::hash<std::string_view>{}(std::string_view(str.data(), str.length()));
    }
};
} // namespace std



