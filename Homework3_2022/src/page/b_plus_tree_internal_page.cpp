/**
 * b_plus_tree_internal_page.cpp
 */
#include <iostream>
#include <sstream>

#include "common/exception.h"
#include "page/b_plus_tree_internal_page.h"

namespace scudb {
/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/
/*
 * Init method after creating a new internal page
 * Including set page type, set current size, set page id, set parent id and set
 * max page size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Init(page_id_t page_id,
                                          page_id_t parent_id) {
    //// 初始化类成员变量
    SetSize(0);
    SetPageId(page_id);
    SetPageType(IndexPageType::INTERNAL_PAGE);
    SetParentPageId(parent_id);
    int size =  (PAGE_SIZE- sizeof(BPlusTreeInternalPage))/sizeof(MappingType);
    SetMaxSize(size - 1);
}
/*
 * Helper method to get/set the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
KeyType B_PLUS_TREE_INTERNAL_PAGE_TYPE::KeyAt(int index) const {
  // replace with your own code
  //// 先判断范围
  if(index >= 0 && index < GetSize()) assert(true);
  return array[index].first;
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetKeyAt(int index, const KeyType &key) {
    if(index >= 0 && index < GetSize()) assert(true);
    array[index].first = key;
}

/*
 * Helper method to find and return array index(or offset), so that its value
 * equals to input "value"
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueIndex(const ValueType &value) const {
    for (int i = 0; i < GetSize(); i++) {
        if (value != ValueAt(i)) continue;
        return i;
    }
    return -1;
}

/*
 * Helper method to get the value associated with input "index"(a.k.a array
 * offset)
 */
INDEX_TEMPLATE_ARGUMENTS
ValueType B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueAt(int index) const {
    if(index >= 0 && index < GetSize()) assert(true);
    return array[index].second;
}

/*****************************************************************************
 * LOOKUP
 *****************************************************************************/
/*
 * Find and return the child pointer(page_id) which points to the child page
 * that contains input "key"
 * Start the search from the second key(the first key should always be invalid)
 */
INDEX_TEMPLATE_ARGUMENTS
ValueType
B_PLUS_TREE_INTERNAL_PAGE_TYPE::Lookup(const KeyType &key,
                                       const KeyComparator &comparator) const {

    assert(GetSize() > 1);////assert错误参数
    ////init value
    int begin = 1;
    int end = GetSize() -1;

    //// 二分查找
    while (begin <= end){
        int mid = (end - begin) / 2 + begin;
        ////target在后半段
        if(comparator(array[mid].first,key) <= 0) begin = mid + 1;

        ////target在前半段
        else end = mid - 1 ;
    }
    return array[begin - 1].second;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Populate new root page with old_value + new_key & new_value
 * When the insertion cause overflow from leaf page all the way upto the root
 * page, you should create a new root page and populate its elements.
 * NOTE: This method is only called within InsertIntoParent()(b_plus_tree.cpp)
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::PopulateNewRoot(
    const ValueType &old_value, const KeyType &new_key,
    const ValueType &new_value) {

    array[0].second = old_value;
    array[1].first = new_key;
    array[1].second = new_value;

    IncreaseSize(1);
}
/*
 * Insert new_key & new_value pair right after the pair with its value ==
 * old_value
 * @return:  new size after insertion
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_INTERNAL_PAGE_TYPE::InsertNodeAfter(
    const ValueType &old_value, const KeyType &new_key,
    const ValueType &new_value) {

    int index = ValueIndex(old_value) + 1;
    assert(index > 0);
    IncreaseSize(1);
    int cur_size = GetSize();
    //// 移动
    for (int i = cur_size - 1; i > index; i--) {
        array[i].first = array[i - 1].first;
        array[i].second = array[i - 1].second;
    }
    ////插入新node
    array[index].first = new_key;
    array[index].second = new_value;
    return cur_size;
}

/*****************************************************************************
 * SPLIT
 *****************************************************************************/
/*
 * Remove half of key & value pairs from this page to "recipient" page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveHalfTo(
    BPlusTreeInternalPage *recipient,
    BufferPoolManager *buffer_pool_manager) {
    assert(recipient != nullptr);
    int total = GetMaxSize() + 1;
    assert(GetSize() == total);
    int mid_index = (total)/2;
    page_id_t recipPageId = recipient->GetPageId();

    for (int i = mid_index; i < total; i++) {
        recipient->array[i - mid_index].first = array[i].first;
        recipient->array[i - mid_index].second = array[i].second;

        ////更新child page的parent page
        auto childRawPage = buffer_pool_manager->FetchPage(array[i].second);
        auto *childTreePage = reinterpret_cast<BPlusTreePage *>(childRawPage->GetData());
        childTreePage->SetParentPageId(recipPageId);
        buffer_pool_manager->UnpinPage(array[i].second,true);
    }
    //设置size
    SetSize(mid_index);
    recipient->SetSize(total - mid_index);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyHalfFrom(
    MappingType *items, int size, BufferPoolManager *buffer_pool_manager) {}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Remove the key & value pair in internal page according to input index(a.k.a
 * array offset)
 * NOTE: store key&value pair continuously after deletion
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Remove(int index) {
    assert(index >= 0 && index < GetSize());
    //// index之后的page向前移动
    for (int i = index + 1; i < GetSize(); i++) {
        array[i - 1] = array[i];
    }
    IncreaseSize(-1);
}

/*
 * Remove the only key & value pair in internal page and return the value
 * NOTE: only call this method within AdjustRoot()(in b_plus_tree.cpp)
 */
INDEX_TEMPLATE_ARGUMENTS
ValueType B_PLUS_TREE_INTERNAL_PAGE_TYPE::RemoveAndReturnOnlyChild() {
    ValueType ret = ValueAt(0);
    IncreaseSize(-1);
    assert(GetSize() == 0);
    return ret;
}
/*****************************************************************************
 * MERGE
 *****************************************************************************/
/*
 * Remove all of key & value pairs from this page to "recipient" page, then
 * update relavent key & value pair in its parent page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveAllTo(
    BPlusTreeInternalPage *recipient, int index_in_parent,
    BufferPoolManager *buffer_pool_manager) {

    int start = recipient->GetSize();
    page_id_t recip_pageid = recipient->GetPageId();

    ////找parent page
    Page *page = buffer_pool_manager->FetchPage(GetParentPageId());
    assert(page != nullptr);
    auto *parent = reinterpret_cast<BPlusTreeInternalPage *>(page->GetData());

    SetKeyAt(0, parent->KeyAt(index_in_parent));
    buffer_pool_manager->UnpinPage(parent->GetPageId(), false);

    for (int i = 0; i < GetSize(); ++i) {
        ////逐个移动
        recipient->array[start + i].first = array[i].first;
        recipient->array[start + i].second = array[i].second;

        ////更新child page的parent page
        auto childRawPage = buffer_pool_manager->FetchPage(array[i].second);
        auto *childTreePage = reinterpret_cast<BPlusTreePage *>(childRawPage->GetData());
        childTreePage->SetParentPageId(recip_pageid);
        buffer_pool_manager->UnpinPage(array[i].second,true);
    }
    ////更新parent page
    recipient->SetSize(start + GetSize());
    assert(recipient->GetSize() <= GetMaxSize());
    SetSize(0);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyAllFrom(
    MappingType *items, int size, BufferPoolManager *buffer_pool_manager) {}

/*****************************************************************************
 * REDISTRIBUTE
 *****************************************************************************/
/*
 * Remove the first key & value pair from this page to tail of "recipient"
 * page, then update relavent key & value pair in its parent page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveFirstToEndOf(
    BPlusTreeInternalPage *recipient,
    BufferPoolManager *buffer_pool_manager) {

    MappingType pair{KeyAt(0), ValueAt(0)};
    IncreaseSize(-1);
    memmove(array, array + 1, static_cast<size_t>(GetSize()*sizeof(MappingType)));
    recipient->CopyLastFrom(pair, buffer_pool_manager);

    page_id_t childPageId = pair.second;
    Page *page = buffer_pool_manager->FetchPage(childPageId);
    assert (page != nullptr);
    auto *child = reinterpret_cast<BPlusTreePage *>(page->GetData());
    child->SetParentPageId(recipient->GetPageId());
    assert(child->GetParentPageId() == recipient->GetPageId());
    buffer_pool_manager->UnpinPage(child->GetPageId(), true);

    page = buffer_pool_manager->FetchPage(GetParentPageId());
    auto *parent = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(page->GetData());
    parent->SetKeyAt(parent->ValueIndex(GetPageId()), array[0].first);
    buffer_pool_manager->UnpinPage(GetParentPageId(), true);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyLastFrom(
    const MappingType &pair, BufferPoolManager *buffer_pool_manager) {
    assert(GetSize() + 1 <= GetMaxSize());
    array[GetSize()] = pair;
    IncreaseSize(1);
}

/*
 * Remove the last key & value pair from this page to head of "recipient"
 * page, then update relavent key & value pair in its parent page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveLastToFrontOf(
    BPlusTreeInternalPage *recipient, int parent_index,
    BufferPoolManager *buffer_pool_manager) {
    MappingType pair {KeyAt(GetSize() - 1),ValueAt(GetSize() - 1)};
    IncreaseSize(-1);
    recipient->CopyFirstFrom(pair, parent_index, buffer_pool_manager);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyFirstFrom(
    const MappingType &pair, int parent_index,
    BufferPoolManager *buffer_pool_manager) {
    assert(GetSize() + 1 < GetMaxSize());
    memmove(array + 1, array, GetSize()*sizeof(MappingType));
    IncreaseSize(1);
    array[0] = pair;
    // update child parent page id
    page_id_t childPageId = pair.second;
    Page *page = buffer_pool_manager->FetchPage(childPageId);
    assert (page != nullptr);
    auto *child = reinterpret_cast<BPlusTreePage *>(page->GetData());
    child->SetParentPageId(GetPageId());
    assert(child->GetParentPageId() == GetPageId());
    buffer_pool_manager->UnpinPage(child->GetPageId(), true);

    //// 更新parent‘spage中的相关键值对
    page = buffer_pool_manager->FetchPage(GetParentPageId());
    auto *parent = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(page->GetData());
    parent->SetKeyAt(parent_index, array[0].first);
    buffer_pool_manager->UnpinPage(GetParentPageId(), true);

}

/*****************************************************************************
 * DEBUG
 *****************************************************************************/
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::QueueUpChildren(
    std::queue<BPlusTreePage *> *queue,
    BufferPoolManager *buffer_pool_manager) {
  for (int i = 0; i < GetSize(); i++) {
    auto *page = buffer_pool_manager->FetchPage(array[i].second);
    if (page == nullptr)
      throw Exception(EXCEPTION_TYPE_INDEX,
                      "all page are pinned while printing");
    BPlusTreePage *node =
        reinterpret_cast<BPlusTreePage *>(page->GetData());
    queue->push(node);
  }
}

INDEX_TEMPLATE_ARGUMENTS
std::string B_PLUS_TREE_INTERNAL_PAGE_TYPE::ToString(bool verbose) const {
  if (GetSize() == 0) {
    return "";
  }
  std::ostringstream os;
  if (verbose) {
    os << "[pageId: " << GetPageId() << " parentId: " << GetParentPageId()
       << "]<" << GetSize() << "> ";
  }

  int entry = verbose ? 0 : 1;
  int end = GetSize();
  bool first = true;
  while (entry < end) {
    if (first) {
      first = false;
    } else {
      os << " ";
    }
    os << std::dec << array[entry].first.ToString();
    if (verbose) {
      os << "(" << array[entry].second << ")";
    }
    ++entry;
  }
  return os.str();
}

// valuetype for internalNode should be page id_t
template class BPlusTreeInternalPage<GenericKey<4>, page_id_t,
                                           GenericComparator<4>>;
template class BPlusTreeInternalPage<GenericKey<8>, page_id_t,
                                           GenericComparator<8>>;
template class BPlusTreeInternalPage<GenericKey<16>, page_id_t,
                                           GenericComparator<16>>;
template class BPlusTreeInternalPage<GenericKey<32>, page_id_t,
                                           GenericComparator<32>>;
template class BPlusTreeInternalPage<GenericKey<64>, page_id_t,
                                           GenericComparator<64>>;
} // namespace scudb
