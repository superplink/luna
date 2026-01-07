#pragma once
#include <type_traits>
#include "index.h"
#include "vector.h"



namespace luna {
    

    
// template <class _Container, class... _Args>
// concept HashableContainerC = UnorderedVectorC<_Container, _Args...> &&
// requires (const _Container& a, typename _Container::index_type index, const _Container::key_type& key) {
//     { a.hash(index) } -> std::convertible_to<size_t>;
//     { a.hash(key) } -> std::convertible_to<size_t>;
//     { a.compare(key, index) } -> std::convertible_to<bool>;
// };



// class BucketChain {
// public:



// private:

//     // Vector<>

// };



// template <class T>
// struct Bucket {
//     Index<T> index;
// };



// template <HashableContainerC _Container>
// class BasicContainerHasher : protected _Container {
// public:

//     using base_type = _Container;
//     using value_type = typename base_type::value_type;
//     using key_type = typename base_type::key_type;
//     using index_type = typename base_type::index_type;
//     using bucket_type = Bucket<value_type>;

//     static constexpr int start_bucket_count = 101;
//     static constexpr int max_depth = 4;
//     static constexpr int growth_coeff = 4;

//     BasicContainerHasher () : base_type() {
//         rehash(start_bucket_count);
//     }

//     // returns a pair with a boolean and index
//     // the boolean is true if the item was inserted, and false if it already exists
//     std::pair<bool, index_type> find_or_insert (const value_type& val) {
//         size_t hash = base_type::hash(val);
//         Index<bucket_type> bucket = get_bucket(hash);
//         index_type index = _buckets[bucket].index;
//         if (index == nullindex) {
//             return std::make_pair(true, _insert_bucket_hint(val, hash, bucket));
//         }
//         bool found = base_type::compare(val, index);
//         while (!found) {
//             if (next_in_bucket(index) == nullindex) {
//                 return std::make_pair(true, _insert_bucket_end(val, index));
//             }
//             index = next_in_bucket(index);
//             found = base_type::compare(val, index);
//         }
//         return std::make_pair(false, index);
//     }


//     index_type insert (const value_type& val) {
//         std::pair<bool, index_type> result = find_or_insert(val);
//         assert(result.first);
//         return result.second;
//     }


//     index_type find_index (const key_type& key) const {
//         Index<bucket_type> bucket = get_bucket(base_type::hash(key));
//         index_type index = _buckets[bucket].index;
//         if (index == nullindex) {
//             return nullindex;
//         }
//         assert((int)index < size());
//         bool found = base_type::compare(key, index);
//         while (!found) {
//             index = next_in_bucket(index);
//             if (index == nullindex) {
//                 return nullindex;
//             }
//             found = base_type::compare(key, index);
//         }
//         return index;
//     }

//     // returns a pointer to item if found, and nullptr if not found
//     value_type* find (const key_type& key) {
//         index_type index = find_index(key);
//         return (index == nullindex) ? nullptr : (&base_type::at(index));
//     }
//     // returns a pointer to item if found, and nullptr if not found
//     const value_type* find (const key_type& key) const {
//         index_type index = find_index(key);
//         return (index == nullindex) ? nullptr : (&base_type::at(index));
//     }

//     value_type& at (index_type index) { return base_type::at(index); }
//     const value_type& at (index_type index) const { return base_type::at(index); }

//     void rehash (int new_bucket_count) {    
//         _buckets.clear();
//         _buckets.resize(new_bucket_count, {nullindex});
//         std::fill(_next.begin(), _next.end(), nullindex);
//         for (index_type i = 0; i < _next.size(); i++) {
//             _append_to_bucket(i, get_bucket(base_type::hash(i)));
//         }
//     }
//     bool maybe_rehash () {
//         if (size() <= bucket_count() * max_depth)
//             return false;
//         rehash(bucket_count() * growth_coeff);
//         return true;
//     }

//     int size () const { return _next.size(); }
//     int bucket_count () const { return _buckets.size(); }

//     Index<bucket_type> get_bucket (size_t hash) const {
//         return hash % bucket_count();
//     }

//     index_type next_in_bucket (index_type index) const {
//         if ((int)index >= _next.size()) {
//             assert(false);
//         }
//         return _next[(int)index];
//     }

// private:

//     void _append_to_bucket (index_type index, Index<bucket_type> bucket) {
//         index_type prev_index = _buckets[bucket].index;
//         if (prev_index == nullindex) {
//             _buckets[bucket].index = index;
//             return;
//         }
//         while (next_in_bucket(prev_index) != nullindex) {
//             prev_index = next_in_bucket(prev_index);
//         }
//         _next[prev_index] = index;
//     }

//     index_type _insert_bucket_hint (const value_type& val, size_t hash, Index<bucket_type> bucket) {
//         if (maybe_rehash()) {
//             bucket = get_bucket(hash);
//         }
//         index_type index = base_type::push_back(val);
//         _next.push_back(nullindex);
//         _append_to_bucket(index, bucket);
//         return index;
//     }

//     index_type _insert_bucket_end (const value_type& val, index_type prev_index) {
//         index_type index = base_type::push_back(val);
//         _next.push_back(nullindex);
//         _next[prev_index] = index;
//         return index;
//     }

//     std::vector<bucket_type> _buckets;
//     std::vector<index_type> _next;

// };







} // namespace luna



