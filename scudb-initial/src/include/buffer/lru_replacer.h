/**
 * lru_replacer.h
 *
 * Functionality: The buffer pool manager must maintain a LRU list to collect
 * all the map that are unpinned and ready to be swapped. The simplest way to
 * implement LRU is a FIFO queue, but remember to dequeue or enqueue map when
 * a page changes from unpinned to pinned, or vice-versa.
 */

#pragma once

#include "buffer/replacer.h"
#include <memory>
#include <unordered_map>
#include <mutex>

using namespace std;
namespace scudb {

template <typename T> class LRUReplacer : public Replacer<T> {
    struct Node {
        Node() {};
        Node(T val) : val(val) {};
        T val;
        shared_ptr<Node> prev;
        shared_ptr<Node> next;
    };
public:
  // do not change public interface
  LRUReplacer();

  ~LRUReplacer();

  void Insert(const T &value);

  bool Victim(T &value);

  bool Erase(const T &value);

  size_t Size();

private:
  // add your member variables here

    mutable std::mutex latch; //insure thread safety
    shared_ptr<Node> head; //head node
    shared_ptr<Node> tail; //tail node
    unordered_map<T, shared_ptr<Node>> map;
};

} // namespace scudb
