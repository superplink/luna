#include <iostream>
#include "luna/vector.h"
#include "luna/sparse-vector.h"
#include "luna/dense-vector.h"
#include "luna/iterator-utils.h"


struct Foo {
    Foo (int _n = 0) : n(_n) {
        std::cout << "constructor " << n << "\n";
    }
    ~Foo () {
        std::cout << "destructor " << n << "\n";
    }

    Foo (const Foo& other) : n(other.n) {
        std::cout << "copy constructor " << n << "\n";
    }
    Foo (Foo&& other) : n(other.n) {
        std::cout << "move constructor " << n << "\n";
    }

    Foo& operator= (const Foo& other) {
        n = other.n;
        std::cout << "copy assign " << n << "\n";
        return *this;
    }
    Foo& operator= (Foo&& other) {
        n = other.n;
        std::cout << "move assign " << n << "\n";
        return *this;
    }

    int n;
};


void test_vector () {
    luna::CompactVector<Foo, 2> vec;
    for (int i = 0; i < 4; i++) {
        vec.emplace_back(i);
    }
    std::cout << "\n";
    vec.pop_back();
    std::cout << "\n";
    vec.resize(3, 5);
    std::cout << "\n";
}


int main () {
    luna::SparseVector<Foo> vec;

    for (int i = 0; i < 10; i++) {
        vec.emplace_back(i);
    }
    vec.remove(3);
    vec.remove(7);
    std::cout << "\n";
    // luna::IPairIterator<int*>::base_value_type
    for (auto [i, foo] : vec.ipairs()) {
        std::cout << i << " " << foo.n << "\n";
    }
    std::cout << "\n";
}



