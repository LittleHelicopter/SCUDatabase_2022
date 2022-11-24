/*
 * extendible_hash.h : implementation of in-memory hash table using extendible
 * hashing
 *
 * Functionality: The buffer pool manager must maintain a page table to be able
 * to quickly map a PageId to its corresponding memory location; or alternately
 * report that the PageId does not match any currently-buffered page.
 */

#pragma once

#include <cstdlib>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <mutex>

#include "hash/hash_table.h"

namespace scudb {

    template <typename K, typename V>
    class ExtendibleHash : public HashTable<K, V> {
    public:
        struct Bucket
        {
            Bucket(int depth) : localDepth(depth){};
            int localDepth;
            std::map<K, V> keyMap;
            std::mutex bucketLatch;
        };
        // constructor
        ExtendibleHash(size_t size);
        ExtendibleHash();
        // helper function to generate hash addressing
        size_t HashKey(const K &key) const;
        // helper function to get global & local depth
        int GetGlobalDepth() const;
        int GetLocalDepth(int bucket_id) const;
        int GetNumBuckets() const;
        // lookup and modifier
        bool Find(const K &key, V &value) override;
        bool Remove(const K &key) override;
        void Insert(const K &key, const V &value) override;

        int getIndex(const K &key) const;

    private:
        // add your own member variables here
        std::vector<std::shared_ptr<Bucket>> buckets;
        int globalDepth;
        size_t bucketSize;
        int bucketNum;
        mutable std::mutex latch;
    };
} // namespace scudb
