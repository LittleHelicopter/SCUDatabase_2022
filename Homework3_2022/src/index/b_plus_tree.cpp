/**
 * b_plus_tree.cpp
 */
#include <iostream>
#include <string>

#include "common/exception.h"
#include "common/logger.h"
#include "common/rid.h"
#include "index/b_plus_tree.h"
#include "page/header_page.h"

namespace scudb {
    using namespace std;
    INDEX_TEMPLATE_ARGUMENTS
    BPLUSTREE_TYPE::BPlusTree(const std::string &name, ////B+tree‘s name
                              BufferPoolManager *buffer_pool_manager, ////缓冲池
                              const KeyComparator &comparator,
                              page_id_t root_page_id) ////tree rootpage号
            : index_name_(name), root_page_id_(root_page_id),
              buffer_pool_manager_(buffer_pool_manager), comparator_(comparator) {}

/*
 * Helper function to decide whether current b+tree is empty
 */
    INDEX_TEMPLATE_ARGUMENTS
    bool BPLUSTREE_TYPE::IsEmpty() const {
        if(root_page_id_ == INVALID_PAGE_ID) return true;

        return false;

    }


/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/*
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */
    INDEX_TEMPLATE_ARGUMENTS
    bool BPLUSTREE_TYPE::GetValue(const KeyType &key,
                                  std::vector<ValueType> &result,
                                  Transaction *transaction) {

        //// 找B+树叶子节点
        B_PLUS_TREE_LEAF_PAGE_TYPE *tar_page = FindLeafPage(key, false, OperationType::READ, transaction);
        if (tar_page == nullptr)
            return false;
        else {
            //find value
            result.resize(1);
            auto ret_val = tar_page->Lookup(key,result[0],comparator_);

            ////unpinPage
            FreePagesInTransaction(false,transaction,tar_page->GetPageId());
            return ret_val;
        }

    }

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert constant key & value pair into b+ tree
 * if current tree is empty, start new tree, update root page id and insert
 * entry, otherwise insert into leaf page.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::Insert(const KeyType &key, const ValueType &value,
                            Transaction *transaction) {
    LockRootPageId(true);
    if (IsEmpty()) {
        StartNewTree(key,value);
        TryUnlockRootPageId(true);
        return true;
    }
    TryUnlockRootPageId(true);
    bool is_success = InsertIntoLeaf(key,value,transaction);
    return is_success;
}
/*
 * Insert constant key & value pair into an empty tree
 * User needs to first ask for new page from buffer pool manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then update b+
 * tree's root page id and insert entry directly into leaf page.
 */
    INDEX_TEMPLATE_ARGUMENTS
    void BPLUSTREE_TYPE::StartNewTree(const KeyType &key, const ValueType &value) {


        page_id_t id;
        Page *root_page = buffer_pool_manager_->NewPage(id);

        auto *root = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(root_page->GetData());

        root->Init(id,INVALID_PAGE_ID);
        root_page_id_ = id;
        UpdateRootPageId(true);

        root->Insert(key,value,comparator_);

        buffer_pool_manager_->UnpinPage(id,true);
    }

/*
 * Insert constant key & value pair into leaf page
 * User needs to first find the right leaf page as insertion target, then look
 * through leaf page to see whether insert key exist or not. If exist, return
 * immdiately, otherwise insert entry. Remember to deal with split if necessary.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */

    INDEX_TEMPLATE_ARGUMENTS
    bool BPLUSTREE_TYPE::InsertIntoLeaf(const KeyType &key, const ValueType &value,
                                        Transaction *transaction) {

        auto *leaf_page = FindLeafPage(key, false, OperationType::INSERT, transaction);
        ValueType v;

        bool exist = leaf_page->Lookup(key,v,comparator_);
        if (exist) {
            FreePagesInTransaction(true,transaction);
            return false;
        } else {
            leaf_page->Insert(key,value,comparator_);

            if (leaf_page->GetSize() > leaf_page->GetMaxSize()) {//insert then split
                auto *new_leaf_page = Split(leaf_page,transaction);
                InsertIntoParent(leaf_page,new_leaf_page->KeyAt(0),new_leaf_page,transaction);
            }

            FreePagesInTransaction(true,transaction);
            return true;
        }

    }

/*
 * Split input page and return newly created page.
 * Using template N to represent either internal page or leaf page.
 * User needs to first ask for new page from buffer pool manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then move half
 * of key & value pairs from input page to newly created page
 */
    INDEX_TEMPLATE_ARGUMENTS
    template <typename N> N *BPLUSTREE_TYPE::Split(N *node, Transaction *transaction) {

        page_id_t new_page_id;
        Page* const new_page = buffer_pool_manager_->NewPage(new_page_id);

        new_page->WLatch();
        transaction->AddIntoPageSet(new_page);

        N *new_node = reinterpret_cast<N *>(new_page->GetData());
        new_node->Init(new_page_id, node->GetParentPageId());
        node->MoveHalfTo(new_node, buffer_pool_manager_);

        return new_node;
    }

/*
 * Insert key & value pair into internal page after split
 * @param   old_node      input page from split() method
 * @param   key
 * @param   new_node      returned page from split() method
 * User needs to first find the parent page of old_node, parent node must be
 * adjusted to take info of new_node into account. Remember to deal with split
 * recursively if necessary.
 */
    INDEX_TEMPLATE_ARGUMENTS
    void BPLUSTREE_TYPE::InsertIntoParent(BPlusTreePage *old_node,
                                          const KeyType &key,
                                          BPlusTreePage *new_node,
                                          Transaction *transaction) {
        assert(old_node != nullptr && new_node != nullptr && transaction != nullptr);
        if (old_node->IsRootPage()) {
            Page* const new_page = buffer_pool_manager_->NewPage(root_page_id_);

            auto *new_root = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(new_page->GetData());
            new_root->Init(root_page_id_);
            new_root->PopulateNewRoot(old_node->GetPageId(),key,new_node->GetPageId());

            old_node->SetParentPageId(root_page_id_);
            new_node->SetParentPageId(root_page_id_);
            UpdateRootPageId();

            //buffer_pool unpin
            buffer_pool_manager_->UnpinPage(new_root->GetPageId(),true);
            return;
        }
        else{
            page_id_t par_id = old_node->GetParentPageId();
            auto *page = FetchPage(par_id);
            auto *parent = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(page);
            new_node->SetParentPageId(par_id);
            parent->InsertNodeAfter(old_node->GetPageId(), key, new_node->GetPageId());

            if (parent->GetSize() > parent->GetMaxSize()) {
            ////插入后超出容量，spilt
                auto *new_leaf_page = Split(parent,transaction);//new page need unpin
                InsertIntoParent(parent,new_leaf_page->KeyAt(0),new_leaf_page,transaction);
            }
            buffer_pool_manager_->UnpinPage(par_id,true);
        }

    }

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Delete key & value pair associated with input key
 * If current tree is empty, return immdiately.
 * If not, User needs to first find the right leaf page as deletion target, then
 * delete entry from leaf page. Remember to deal with redistribute or merge if
 * necessary.
 */
    INDEX_TEMPLATE_ARGUMENTS
    void BPLUSTREE_TYPE::Remove(const KeyType &key, Transaction *transaction) {
        ////空则直接return
        if (IsEmpty()) return;
        else{
            ////以Delete模式寻找target page
            auto *tar = FindLeafPage(key, false, OperationType::DELETE, transaction);
            int cur_size = tar->RemoveAndDeleteRecord(key,comparator_);
            if (cur_size < tar->GetMinSize()) {
                CoalesceOrRedistribute(tar,transaction);
            }
            FreePagesInTransaction(true,transaction);
        }

    }

/*
 * User needs to first find the sibling of input page. If sibling's size + input
 * page's size > page's max size, then redistribute. Otherwise, merge.
 * Using template N to represent either internal page or leaf page.
 * @return: true means target leaf page should be deleted, false means no
 * deletion happens
 */
    INDEX_TEMPLATE_ARGUMENTS
    template <typename N>
    bool BPLUSTREE_TYPE::CoalesceOrRedistribute(N *node, Transaction *transaction) {

        if (node->IsRootPage()) {
            bool delOldRoot = AdjustRoot(node);
            if (delOldRoot) {transaction->AddIntoDeletedPageSet(node->GetPageId());}
            return delOldRoot;
        }

        N *nearNode;
        bool is_r_sibling = FindLeftSibling(node, nearNode, transaction);
        BPlusTreePage *parent = FetchPage(node->GetParentPageId());
        auto *parent_page = static_cast<B_PLUS_TREE_INTERNAL_PAGE *>(parent);


        if (node->GetSize() + nearNode->GetSize() <= node->GetMaxSize()) {
            if (is_r_sibling) {swap(node, nearNode);}
            int remove_index = parent_page->ValueIndex(node->GetPageId());
            ////调用合并函数，合并
            Coalesce(nearNode, node, parent_page, remove_index, transaction);
            buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), true);
            return true;
        }else {
            int index_in_parent = parent_page->ValueIndex(node->GetPageId());
            Redistribute(nearNode, node, index_in_parent);
            buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), false);
            return false;
        }

    }

    INDEX_TEMPLATE_ARGUMENTS
    template <typename N>
    bool BPLUSTREE_TYPE::FindLeftSibling(N *node, N * &sibling, Transaction *transaction) {
        auto page = FetchPage(node->GetParentPageId());
        auto *parent = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(page);
        int index = parent->ValueIndex(node->GetPageId());

        int siblingIndex = index - 1;
        ////无左兄弟，找右兄弟
        if (index == 0) {
            siblingIndex = index + 1;
        }
        ////调用函数寻找兄弟
        sibling = reinterpret_cast<N *>(CrabingProtocalFetchPage(
                parent->ValueAt(siblingIndex), OperationType::DELETE, -1, transaction));
        buffer_pool_manager_->UnpinPage(parent->GetPageId(), false);
        if(index == 0){
            ////返回true，表示是右兄弟
            return true;
        }else {
            return false;
        }
    }

/*
 * Move all the key & value pairs from one page to its sibling page, and notify
 * buffer pool manager to delete this page. Parent page must be adjusted to
 * take info of deletion into account. Remember to deal with coalesce or
 * redistribute recursively if necessary.
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 * @param   parent             parent page of input "node"
 * @return  true means parent node should be deleted, false means no deletion
 * happend
 */
    INDEX_TEMPLATE_ARGUMENTS
    template <typename N>
    bool BPLUSTREE_TYPE::Coalesce(
            N *&neighbor_node, N *&node,
            BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *&parent,
            int index, Transaction *transaction) {


        node->MoveAllTo(neighbor_node,index,buffer_pool_manager_);
        transaction->AddIntoDeletedPageSet(node->GetPageId());
        parent->Remove(index);
        if (parent->GetSize() <= parent->GetMinSize()) {
            return CoalesceOrRedistribute(parent,transaction);
        } else {
            return false;
        }
    }

/*
 * Redistribute key & value pairs from one page to its sibling page. If index ==
 * 0, move sibling page's first key & value pair into end of input "node",
 * otherwise move sibling page's last key & value pair into head of input
 * "node".
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 */
    INDEX_TEMPLATE_ARGUMENTS
    template <typename N>
    void BPLUSTREE_TYPE::Redistribute(N *neighbor_node, N *node, int index) {
        if (index == 0) neighbor_node->MoveFirstToEndOf(node,buffer_pool_manager_);
        else    neighbor_node->MoveLastToFrontOf(node, index, buffer_pool_manager_);
    }
/*
 * Update root page if necessary
 * NOTE: size of root page can be less than min size and this method is only
 * called within coalesceOrRedistribute() method
 * case 1: when you delete the last element in root page, but root page still
 * has one last child
 * case 2: when you delete the last element in whole b+ tree
 * @return : true means root page should be deleted, false means no deletion
 * happend
 */
    INDEX_TEMPLATE_ARGUMENTS
    bool BPLUSTREE_TYPE::AdjustRoot(BPlusTreePage *old_root_node) {
        ////更新root节点
        if (old_root_node->IsLeafPage()) {// case 2
            root_page_id_ = INVALID_PAGE_ID;
            UpdateRootPageId();
            return true;
        }
        if (old_root_node->GetSize() == 1) {// case 1
            auto *root = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(old_root_node);
            const page_id_t newRootId = root->RemoveAndReturnOnlyChild();
            root_page_id_ = newRootId;
            UpdateRootPageId();
            Page *page = buffer_pool_manager_->FetchPage(newRootId);
            assert(page != nullptr);
            auto *new_root =
                    reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(page->GetData());
            new_root->SetParentPageId(INVALID_PAGE_ID);
            buffer_pool_manager_->UnpinPage(newRootId, true);
            return true;
        }
        return false;
    }

/*****************************************************************************
 * INDEX ITERATOR
 *****************************************************************************/
/*
 * Input parameter is void, find the leaftmost leaf page first, then construct
 * index iterator
 * @return : index iterator
 */
    INDEX_TEMPLATE_ARGUMENTS
    INDEXITERATOR_TYPE BPLUSTREE_TYPE::Begin() {
        KeyType unuse{};
        auto start_leaf = FindLeafPage(unuse, true);
        TryUnlockRootPageId(false);
        return INDEXITERATOR_TYPE(start_leaf, 0, buffer_pool_manager_);
    }

/*
 * Input parameter is low key, find the leaf page that contains the input key
 * first, then construct index iterator
 * @return : index iterator
 */
    INDEX_TEMPLATE_ARGUMENTS
    INDEXITERATOR_TYPE BPLUSTREE_TYPE::Begin(const KeyType &key) {
        ////寻找index
        auto start_leaf = FindLeafPage(key);
        TryUnlockRootPageId(false);
        if (start_leaf == nullptr) {
            ////没找到，则返回0
            return INDEXITERATOR_TYPE(start_leaf, 0, buffer_pool_manager_);
        }
        int idx = start_leaf->KeyIndex(key,comparator_);
        ////找到了，构造idx的index iterator
        return INDEXITERATOR_TYPE(start_leaf, idx, buffer_pool_manager_);//return
    }

/*****************************************************************************
 * UTILITIES AND DEBUG
 *****************************************************************************/
/*
 * Find leaf page containing particular key, if leftMost flag == true, find
 * the left most leaf page
 */
    INDEX_TEMPLATE_ARGUMENTS
    B_PLUS_TREE_LEAF_PAGE_TYPE *BPLUSTREE_TYPE::FindLeafPage(const KeyType &key,
                                                             bool leftMost, OperationType op,
                                                             Transaction *transaction) {
        bool exclusive = (op != OperationType::READ);
        LockRootPageId(exclusive);
        if (IsEmpty()) {
            TryUnlockRootPageId(exclusive);
            return nullptr;
        }
        auto pointer = CrabingProtocalFetchPage(root_page_id_,op,-1,transaction);
        page_id_t next;
        for (page_id_t cur = root_page_id_;
            !pointer->IsLeafPage();
            pointer = CrabingProtocalFetchPage(next,op,cur,transaction),cur = next) {
            ////for 遍历
            auto *internalPage = static_cast<B_PLUS_TREE_INTERNAL_PAGE *>(pointer);
            if (leftMost) {
                next = internalPage->ValueAt(0);
            }else {
                next = internalPage->Lookup(key,comparator_);
            }
        }
        return static_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(pointer);
    }
    INDEX_TEMPLATE_ARGUMENTS
    BPlusTreePage *BPLUSTREE_TYPE::FetchPage(page_id_t page_id) {
        auto page = buffer_pool_manager_->FetchPage(page_id);
        return reinterpret_cast<BPlusTreePage *>(page->GetData());
    }
    INDEX_TEMPLATE_ARGUMENTS
    BPlusTreePage *BPLUSTREE_TYPE::CrabingProtocalFetchPage(page_id_t page_id, OperationType op, page_id_t previous, Transaction *transaction) {
        bool exclusive = op != OperationType::READ;
        auto page = buffer_pool_manager_->FetchPage(page_id);////获取page
        Lock(exclusive,page);
        auto tree_page = reinterpret_cast<BPlusTreePage *>(page->GetData());
        if (previous > 0 && (!exclusive || tree_page->isSafe(op))) {
            FreePagesInTransaction(exclusive,transaction,previous);
        }
        if (transaction != nullptr)
            transaction->AddIntoPageSet(page);
        return tree_page;
    }

    INDEX_TEMPLATE_ARGUMENTS
    void BPLUSTREE_TYPE::FreePagesInTransaction(bool exclusive, Transaction *transaction, page_id_t cur) {
        TryUnlockRootPageId(exclusive);
        if (transaction == nullptr) {
            assert(!exclusive && cur >= 0);
            Unlock(false,cur);
            buffer_pool_manager_->UnpinPage(cur,false);
            return;
        }
        ////不为空
        for (Page *page : *transaction->GetPageSet()) {
            int cur_pid = page->GetPageId();
            Unlock(exclusive,page);
            buffer_pool_manager_->UnpinPage(cur_pid,exclusive);

            if (transaction->GetDeletedPageSet()->find(cur_pid) != transaction->GetDeletedPageSet()->end()) {
                buffer_pool_manager_->DeletePage(cur_pid);////则删除
                transaction->GetDeletedPageSet()->erase(cur_pid);
            }
        }
        assert(transaction->GetDeletedPageSet()->empty());
        transaction->GetPageSet()->clear();
    }

/*
 * Update/Insert root page id in header page(where page_id = 0, header_page is
 * defined under include/page/header_page.h)
 * Call this method everytime root page id is changed.
 * @parameter: insert_record      defualt value is false. When set to true,
 * insert a record <index_name, root_page_id> into header page instead of
 * updating it.
 */
    INDEX_TEMPLATE_ARGUMENTS
    void BPLUSTREE_TYPE::UpdateRootPageId(int insert_record) {
        auto *header_page = static_cast<HeaderPage *>(
                buffer_pool_manager_->FetchPage(HEADER_PAGE_ID));
        if (insert_record){

            header_page->InsertRecord(index_name_, root_page_id_);
        } else{

            header_page->UpdateRecord(index_name_, root_page_id_);
        }
        buffer_pool_manager_->UnpinPage(HEADER_PAGE_ID, true);
    }

/*
 * This method is used for debug only
 * print out whole b+tree sturcture, rank by rank
 */
    INDEX_TEMPLATE_ARGUMENTS
    std::string BPLUSTREE_TYPE::ToString(bool verbose) {
        if (IsEmpty()) {
            return "Empty tree";
        }
        std::queue<BPlusTreePage *> todo, tmp;
        std::stringstream tree;
        auto node = reinterpret_cast<BPlusTreePage *>(
                buffer_pool_manager_->FetchPage(root_page_id_));
        if (node == nullptr) {
            throw Exception(EXCEPTION_TYPE_INDEX,
                            "all page are pinned while printing");
        }
        todo.push(node);
        bool first = true;
        while (!todo.empty()) {
            node = todo.front();
            if (first) {
                first = false;
                tree << "| ";
            }
            // leaf page, print all key-value pairs
            if (node->IsLeafPage()) {
                auto page = reinterpret_cast<BPlusTreeLeafPage<KeyType, ValueType, KeyComparator> *>(node);
                tree << page->ToString(verbose) <<"("<<node->GetPageId()<< ")| ";
            } else {
                auto page = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(node);
                tree << page->ToString(verbose) <<"("<<node->GetPageId()<< ")| ";
                page->QueueUpChildren(&tmp, buffer_pool_manager_);
            }
            todo.pop();
            if (todo.empty() && !tmp.empty()) {
                todo.swap(tmp);
                tree << '\n';
                first = true;
            }
            // unpin node when we are done
            buffer_pool_manager_->UnpinPage(node->GetPageId(), false);
        }
        return tree.str();
    }

/*
 * This method is used for test only
 * Read data from file and insert one by one
 */
    INDEX_TEMPLATE_ARGUMENTS
    void BPLUSTREE_TYPE::InsertFromFile(const std::string &file_name,
                                        Transaction *transaction) {
        int64_t key;
        std::ifstream input(file_name);
        while (input) {
            input >> key;
            KeyType index_key;
            index_key.SetFromInteger(key);
            RID rid(key);
            Insert(index_key, rid, transaction);
        }
    }
/*
 * This method is used for test only
 * Read data from file and remove one by one
 */
    INDEX_TEMPLATE_ARGUMENTS
    void BPLUSTREE_TYPE::RemoveFromFile(const std::string &file_name,
                                        Transaction *transaction) {
        int64_t key;
        std::ifstream input(file_name);
        while (input) {
            input >> key;
            KeyType index_key;
            index_key.SetFromInteger(key);
            Remove(index_key, transaction);
        }
    }


/***************************************************************************
 *  Check integrity of B+ tree data structure.
 ***************************************************************************/

    ////判断是否是平衡树
    INDEX_TEMPLATE_ARGUMENTS
    int BPLUSTREE_TYPE::isBalanced(page_id_t pid) {
        if (IsEmpty()) return true;
        ////通过pid获取目标page
        auto node = reinterpret_cast<BPlusTreePage *>(buffer_pool_manager_->FetchPage(pid));
        int ret = 0;
        if (!node->IsLeafPage())  {
            auto page = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(node);
            int last = -2;
            ////遍历
            for (int i = 0; i < page->GetSize(); i++) {
                //递归判断
                int cur = isBalanced(page->ValueAt(i));
                if (cur >= 0 && last == -2) {
                    last = cur;
                    ret = last + 1;
                }else if (last != cur) {
                    ret = -1;
                    break;
                }
            }
        }
        ////解锁
        buffer_pool_manager_->UnpinPage(pid,false);
        return ret;
    }

    INDEX_TEMPLATE_ARGUMENTS
    bool BPLUSTREE_TYPE::isPageCorr(page_id_t pid,pair<KeyType,KeyType> &out) {
        if (IsEmpty()) return true;
        else{
            auto node = reinterpret_cast<BPlusTreePage *>(buffer_pool_manager_->FetchPage(pid));
            if (node == nullptr) {
                throw Exception(EXCEPTION_TYPE_INDEX,"all page are pinned while isPageCorr");
            }
            bool ret = true;
            if (node->IsLeafPage())  {
                auto page = reinterpret_cast<BPlusTreeLeafPage<KeyType, ValueType, KeyComparator> *>(node);
                int size = page->GetSize();
                ret = ret && (size >= node->GetMinSize() && size <= node->GetMaxSize());
                for (int i = 1; i < size; i++) {
                    if (comparator_(page->KeyAt(i-1), page->KeyAt(i)) > 0) {
                        ret = false;
                        break;
                    }
                }
                out = pair<KeyType,KeyType>{page->KeyAt(0),page->KeyAt(size-1)};
            } else {
                auto page = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(node);
                int size = page->GetSize();
                ret = ret && (size >= node->GetMinSize() && size <= node->GetMaxSize());
                pair<KeyType,KeyType> left,right;
                for (int i = 1; i < size; i++) {
                    if (i == 1) {
                        ret = ret && isPageCorr(page->ValueAt(0),left);
                    }
                    ret = ret && isPageCorr(page->ValueAt(i),right);
                    ret = ret && (comparator_(page->KeyAt(i) ,left.second)>0 && comparator_(page->KeyAt(i), right.first)<=0);
                    ret = ret && (i == 1 || comparator_(page->KeyAt(i-1) , page->KeyAt(i)) < 0);
                    if (!ret) break;
                    left = right;
                }
                out = pair<KeyType,KeyType>{page->KeyAt(0),page->KeyAt(size-1)};
            }
            buffer_pool_manager_->UnpinPage(pid,false);
            return ret;
        }

    }


    INDEX_TEMPLATE_ARGUMENTS
    bool BPLUSTREE_TYPE::Check(bool forceCheck) {
        if (!forceCheck && !openCheck) {
            return true;
        }
        pair<KeyType,KeyType> in;
        bool isPageInOrderAndSizeCorr = isPageCorr(root_page_id_, in);
        bool isBal = (isBalanced(root_page_id_) >= 0);
        bool isAllUnpin = buffer_pool_manager_->CheckAllUnpined();
        if (!isPageInOrderAndSizeCorr) cout<<"problem in page order or page size"<<endl;
        if (!isBal) cout<<"problem in balance"<<endl;
        if (!isAllUnpin) cout<<"problem in page unpin"<<endl;
        return isPageInOrderAndSizeCorr && isBal && isAllUnpin;
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    thread_local int BPlusTree<KeyType, ValueType, KeyComparator>::mRootLockedCnt = 0;


    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BPlusTree<KeyType, ValueType, KeyComparator>::Lock(bool exclusive, Page *page) {
        if (exclusive) {
            page->WLatch();
        } else {
            page->RLatch();
        }
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BPlusTree<KeyType, ValueType, KeyComparator>::Unlock(bool exclusive, Page *page) {
        if (exclusive) {
            page->WUnlatch();
        } else {
            page->RUnlatch();
        }
    }


    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BPlusTree<KeyType, ValueType, KeyComparator>::Unlock(bool exclusive, page_id_t pageId) {
        auto page = buffer_pool_manager_->FetchPage(pageId);////拉起，保护page
        Unlock(exclusive,page);//解锁
        buffer_pool_manager_->UnpinPage(pageId,exclusive);////在释放

    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BPlusTree<KeyType, ValueType, KeyComparator>::LockRootPageId(bool exclusive) {
        if (exclusive) {
            mMutex_.WLock();
        } else {
            mMutex_.RLock();
        }
        mRootLockedCnt++;
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BPlusTree<KeyType, ValueType, KeyComparator>::TryUnlockRootPageId(bool exclusive) {
        if (mRootLockedCnt > 0) {
            if (exclusive) {
                mMutex_.WUnlock();
            } else {
                mMutex_.RUnlock();
            }
            mRootLockedCnt--;
        }
    }


    template class BPlusTree<GenericKey<4>, RID, GenericComparator<4>>;
    template class BPlusTree<GenericKey<8>, RID, GenericComparator<8>>;
    template class BPlusTree<GenericKey<16>, RID, GenericComparator<16>>;
    template class BPlusTree<GenericKey<32>, RID, GenericComparator<32>>;
    template class BPlusTree<GenericKey<64>, RID, GenericComparator<64>>;
} // namespace scudb
