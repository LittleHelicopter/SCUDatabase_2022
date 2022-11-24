/**
 * LRU implementation
 */
#include "buffer/lru_replacer.h"
#include "page/page.h"

namespace scudb {

template <typename T> LRUReplacer<T>::LRUReplacer() {
    head = make_shared<Node>();
    tail = make_shared<Node>();
    head->next = tail;
    tail->prev = head;
}

/*
 * In this task, I am peimitted that I could assume I will not run out of memory
 * So there is no 'actual' implement with deconstructor
 */
template <typename T> LRUReplacer<T>::~LRUReplacer() {}

/*
 * Insert value into LRU
 */
template <typename T> void LRUReplacer<T>::Insert(const T &value) {
    std::lock_guard<std::mutex> lck(latch);
    shared_ptr<Node> cur;
    if(map.find(value) == map.end())
        cur = make_shared<Node>(value);
    else {
        cur = map[value];
        shared_ptr<Node> pre = cur->prev;
        shared_ptr<Node> nxt = cur->next;
        pre->next = nxt;
        nxt->prev = pre;
    }
    //最新的节点放到队首
    shared_ptr<Node> first = head->next;
    cur->next = first;
    first->prev = cur;
    cur->prev = head;
    head->next = cur;
    map[value] = cur;
    return;
}

/* If LRU is non-empty, pop the head member from LRU to argument "value", and
 * return true. If LRU is empty, return false
 */
template <typename T> bool LRUReplacer<T>::Victim(T &value) {
    std::lock_guard<std::mutex> lck(latch);
    // LRU为空， return false
    if(map.empty()) return false;

    shared_ptr<Node> last = tail->prev;
    tail->prev = last->prev;
    last->prev->next = tail;
    value = last->val;

    map.erase(last->val);
    return true;
}

/*
 * Remove value from LRU. If removal is successful, return true, otherwise
 * return false
 */
template <typename T> bool LRUReplacer<T>::Erase(const T &value) {
    std::lock_guard<std::mutex> lck(latch);
    if (map.find(value) != map.end()) {
        shared_ptr<Node> cur = map[value];
        cur->prev->next = cur->next;
        cur->next->prev = cur->prev;
    }
    return map.erase(value);
}

template <typename T> size_t LRUReplacer<T>::Size() { 
  std::lock_guard<std::mutex> lck(latch);
  return map.size();
}

template class LRUReplacer<Page *>;
// test only
template class LRUReplacer<int>;

} // namespace scudb
