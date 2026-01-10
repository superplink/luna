#include <iostream>
#include "luna/vector.h"
#include "luna/sparse-vector.h"
#include "luna/dense-vector.h"
#include "luna/iterator-utils.h"
#include "luna/set.h"
#include "luna/map.h"
#include "benchmark.h"
#include "luna/vector-stack.h"
#include <unordered_map>


using namespace luna;


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


struct StupidlyBigObject {
    int n[1];
};


void test_unordered_vectors () {
    SparseVector<int> vec1;
    DenseVector<int> vec2;
    int count = 100000000;

    int n1 = 0;
    int n2 = 0;

    log_time_action([&]{
        for (int i = 0; i < count; i++) {
            vec1.push_back(i);
        }
    });
    log_time_action([&]{
        for (int i = 0; i < count; i++) {
            vec2.push_back(i);
        }
    });
    std::cout << "\n";

    log_time_action([&]{
        for (int i = 0; i < count; i++) {
            n1 += vec1[i];
        }
    });
    log_time_action([&]{
        for (int i = 0; i < count; i++) {
            n2 += vec2[i];
        }
    });
    std::cout << "\n";

    log_time_action([&]{
        for (int i = 0; i < count; i += 2) {
            vec1.remove(i);
        }
    });
    log_time_action([&]{
        for (int i = 0; i < count; i += 2) {
            vec2.remove(i);
        }
    });
    std::cout << "\n";

    log_time_action([&]{
        for (int n : vec1) {
            n1 -= n;
        }
    });
    log_time_action([&]{
        for (int n : vec2) {
            n2 -= n;
        }
    });
    std::cout << "\n";

    std::cout << n1 << " " << n2 << "\n";
}


void time_vector () {
    Vector<int> vec1;
    std::vector<int> vec2;

    int count = 100000000;

    int n1 = 0;
    int n2 = 0;

    log_time_action([&]{
        for (int i = 0; i < count; i++) {
            vec1.push_back(i);
        }
    });
    log_time_action([&]{
        for (int i = 0; i < count; i++) {
            vec2.push_back(i);
        }
    });

    log_time_action([&]{
        for (int i = 0; i < count; i++) {
            n1 += vec1[i];
        }
    });
    log_time_action([&]{
        for (int i = 0; i < count; i++) {
            n2 += vec2[i];
        }
    });

    std::cout << n1 << " " << n2 << "\n";
}


void test_sparse_vector () {
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


void test_dense_vector () {
    luna::DenseVector<Foo> vec;

    for (int i = 0; i < 10; i++) {
        vec.emplace_back(i);
    }
    vec.remove(3);
    vec.remove(7);
    std::cout << "\n";
    for (auto [i, foo] : vec.ipairs()) {
        std::cout << i << " " << foo.n << "\n";
    }
    std::cout << "\n";
}


void test_map () {
    Map<int, StupidlyBigObject> map1;
    std::unordered_map<int, StupidlyBigObject> map2;

    int count = 100000000;

    log_time_action([&]{
        for (int i = 0; i < count; i++) {
            map1.insert(i, StupidlyBigObject{ {i} });
        }
    });
    log_time_action([&]{
        for (int i = 0; i < count; i++) {
            map2.insert(std::make_pair(i, StupidlyBigObject{ {i} }));
        }
    });
    std::cout << "\n";

    int n1 = 0;
    int n2 = 0;

    log_time_action([&]{
        for (int i = 0; i < count; i++) {
            n1 += map1.at(i).n[0];
        }
    });
    log_time_action([&]{
        for (int i = 0; i < count; i++) {
            n2 += map2[i].n[0];
        }
    });
    std::cout << "\n";

    log_time_action([&]{
        for (int i = 0; i < count; i += 2) {
            map1.remove(i);
        }
    });
    log_time_action([&]{
        for (int i = 0; i < count; i += 2) {
            map2.erase(i);
        }
    });
    std::cout << "\n";

    log_time_action([&]{
        for (auto [key, val] : map1) {
            n1 -= val.n[0];
        }
    });
    log_time_action([&]{
        for (auto [key, val] : map2) {
            n2 -= val.n[0];
        }
    });
    std::cout << "\n";

    std::cout << n1 << " " << n2 << "\n";
}


void test_vector_stack () {
    VectorStack<int> vec;
    vec.push_vector();
    for (int i = 0; i < 4; i++) {
        vec.push_back(i);
    }
    vec.push_vector();
    for (int i = 0; i < 10; i++) {
        vec.push_back(i);
    }
    vec.push_vector();
    for (int i = 0; i < 2; i++) {
        vec.push_back(i);
    }
    vec.push_vector();
    for (int i = 0; i < 6; i++) {
        vec.push_back(i);
    }

    for (auto row : vec) {
        for (int n : row) {
            std::cout << n << " ";
        }
        std::cout << "\n";
    }
}


void test_vector () {
    Vector<int> vec;
    for (int i = 0; i < 10; i++) {
        vec.push_back(i);
    }
    for (int n : vec) {
        std::cout << n << "\n";
    }
    std::cout << "\n";
    vec.remove_ordered(4, 6);
    for (int n : vec) {
        std::cout << n << "\n";
    }
    std::cout << "\n";
}


template <GenericChunkC<int, float> T>
void asdf () {
    using chunk_type = GenericChunkType<T, int>;
}


int main () {
    // test_map();
    // test_unordered_vectors();
    // test_vector_stack();
    // using a = ArrayChunkType
    // asdf<GenericHeapChunk>();
    test_vector();
}



