#include <cmath>
#include <list>
#include <algorithm>
#include "hash/extendible_hash.h"
#include "page/page.h"

namespace scudb {

/*
 * constructor
 * array_size: fixed array size for each bucket
 */
    template <typename K, typename V>
    ExtendibleHash<K, V>::ExtendibleHash(size_t size) : globalDepth(0), bucketSize(size),bucketNum(1){
    buckets.push_back(std::make_shared<Bucket>(0));
    }
    template<typename K, typename V>
    ExtendibleHash<K, V>::ExtendibleHash() {
        ExtendibleHash(64);
    }
/*
 * helper function to calculate the hashing address of input key
 */
    template <typename K, typename V>
    size_t ExtendibleHash<K, V>::HashKey(const K &key) const{
        return std::hash<K>{}(key);
    }

/*
 * helper function to return global depth of hash table
 * NOTE: you must implement this function in order to pass test
 */
    template <typename K, typename V>
    int ExtendibleHash<K, V>::GetGlobalDepth() const {
        std::lock_guard<std::mutex> gLock(latch);
        return this->globalDepth;
    }

/*
 * helper function to return local depth of one specific bucket
 * NOTE: you must implement this function in order to pass test
 */
    template <typename K, typename V>
    int ExtendibleHash<K, V>::GetLocalDepth(int bucket_id) const {
        if (buckets[bucket_id]) {
            std::shared_ptr<Bucket> cur = buckets[bucket_id];
            std::lock_guard<std::mutex> bLock(cur->bucketLatch);
            if (cur->keyMap.size() != 0)
                return cur->localDepth;
        }
        return -1;
    }

/*
 * helper function to return current number of bucket in hash table
 */
    template <typename K, typename V>
    int ExtendibleHash<K, V>::GetNumBuckets() const {
        std::lock_guard<std::mutex> gLock(latch);
        return this->bucketNum;
    }

    template <typename K, typename V>
    int ExtendibleHash<K, V>::getIndex(const K &key) const{
        std::lock_guard<std::mutex> gLock(latch);
        return HashKey(key) & ((1 << globalDepth) - 1);
    }

/*
 * lookup function to find value associate with input key
 */
    template <typename K, typename V>
    bool ExtendibleHash<K, V>::Find(const K &key, V &value) {
        int index = getIndex(key);
        std::shared_ptr<Bucket> cur = buckets[index];
        std::lock_guard<std::mutex> bLock(cur->bucketLatch);
        if (cur->keyMap.find(key) != cur->keyMap.end()) {
            value = cur->keyMap[key];
            return true;
        }
        return false;
    }

/*
 * delete <key,value> entry in hash table
 * Shrink & Combination is not required for this project
 */
    template <typename K, typename V>
    bool ExtendibleHash<K, V>::Remove(const K &key) {
        int index = getIndex(key);
        std::shared_ptr<Bucket> cur = buckets[index];
        std::lock_guard<std::mutex> lck(cur->bucketLatch);
        if (cur->keyMap.find(key) != cur->keyMap.end()) {
            cur->keyMap.erase(key);
            return true;
        }
        return false;
    }

/*
 * insert <key,value> entry in hash table
 * Split & Redistribute bucket when there is overflow and if necessary increase
 * global depth
 */
    template <typename K, typename V>
    void ExtendibleHash<K, V>::Insert(const K &key, const V &value) {
        int curIndex = getIndex(key);
        auto cur = buckets[curIndex];
        while (true) {
            std::lock_guard<std::mutex> bLock(cur->bucketLatch);
            if (cur->keyMap.find(key) != cur->keyMap.end() || cur->keyMap.size() < bucketSize) {
                cur->keyMap[key] = value;
                break;
            }
            int mask = (1 << (cur->localDepth)); // 桶大小不足需要分裂0, 1开头不同的桶
            cur->localDepth++;

            {
                std::lock_guard<std::mutex> gLock(latch);
                if (cur->localDepth > globalDepth) {
                    size_t length = buckets.size();
                    for (size_t i = 0; i < length; i++) {
                        buckets.push_back(buckets[i]);
                    }
                    globalDepth++;
                }

                bucketNum++;
                auto newBucket = std::make_shared<Bucket>(cur->localDepth);
                typename std::map<K, V>::iterator it;
                for (it = cur->keyMap.begin(); it != cur->keyMap.end();) {
                    if (HashKey(it->first) & mask) {
                        newBucket->keyMap[it->first] = it->second; //  将localDepth下一位上为0与为1的分开
                        it = cur->keyMap.erase(it);
                    } else it++;
                }
                for (size_t i = 0; i < buckets.size(); i++) {
                    if (buckets[i] == cur && (i & mask))
                        buckets[i] = newBucket;
                }
            }

                curIndex = getIndex(key);
                cur = buckets[curIndex];

        }
    }
    template class ExtendibleHash<page_id_t, Page *>;
    template class ExtendibleHash<Page *, std::list<Page *>::iterator>;
// test purpose
    template class ExtendibleHash<int, std::string>;
    template class ExtendibleHash<int, std::list<int>::iterator>;
    template class ExtendibleHash<int, int>;
} // namespace scudb
