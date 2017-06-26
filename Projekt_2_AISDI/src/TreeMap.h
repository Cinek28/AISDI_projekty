#ifndef AISDI_MAPS_TREEMAP_H
#define AISDI_MAPS_TREEMAP_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>

namespace aisdi {

    template<typename KeyType, typename ValueType>
    class TreeMap {
    public:
        using key_type = KeyType;
        using mapped_type = ValueType;
        using value_type = std::pair<const key_type, mapped_type>;
        using size_type = std::size_t;
        using reference = value_type&;
        using const_reference = const value_type&;

        class ConstIterator;

        class Iterator;

        class TreeNode;

        using iterator = Iterator;
        using const_iterator = ConstIterator;

        TreeMap() : mRoot(nullptr), mCount(0) {}

        TreeMap(std::initializer_list<value_type> list) : TreeMap() {
            for (auto&& item : list)
                insert(item);
        }

        TreeMap(const TreeMap& other) : TreeMap() {
            for (auto&& item : other)
                insert(item);
        }

        TreeMap(TreeMap&& other) : TreeMap() {
            std::swap(mRoot, other.mRoot);
            std::swap(mCount, other.mCount);
        }

        ~TreeMap() {
            clear(mRoot);
        }

        TreeMap& operator=(const TreeMap& other) {
            if (*this == other)
                return *this;
            clear(mRoot);
            for (auto&& item : other)
                insert(item);
            return *this;
        }

        TreeMap& operator=(TreeMap&& other) {
            if (*this == other)
                return *this;
            clear(mRoot);
            std::swap(mRoot, other.mRoot);
            std::swap(mCount, other.mCount);
            return *this;
        }

        bool isEmpty() const {
            return mCount == 0;
        }

        mapped_type& operator[](const key_type& key) {
            TreeNode* node = findNode(key);
            if (node != nullptr)
                return node->mPair.second;
            node = allocate(key);
            return node->mPair.second;
        }

        const mapped_type& valueOf(const key_type& key) const {
            TreeNode* node = findNode(key);
            if (node == nullptr)
                throw std::out_of_range("Key not found.");
            else
                return node->mPair.second;
        }

        mapped_type& valueOf(const key_type& key) {
            return const_cast<mapped_type&>(static_cast<const TreeMap<KeyType, ValueType>*>(this)->valueOf(key));
        }

        const_iterator find(const key_type& key) const {
            TreeNode* node = findNode(key);
            if (node == nullptr)
                return end();
            return Iterator(*this, node, false);
        }

        iterator find(const key_type& key) {
            return static_cast<const TreeMap<KeyType, ValueType>*>(this)->find(key);
        }

        void remove(const key_type& key) {
            removeNode(key);
        }

        void remove(const const_iterator& it) {
            if (it == end())
                throw std::out_of_range("Removing end iterator");
            removeNode(it.mNode->mPair.first);
        }

        size_type getSize() const {
            return mCount;
        }

        bool operator==(const TreeMap& other) const {
            if (mCount != other.mCount)
                return false;

            for (auto&& item : other) {
                TreeNode* node = findNode(item.first);
                if (node == nullptr || node->mPair.second != item.second)
                    return false;
            }

            return true;
        }

        bool operator!=(const TreeMap& other) const {
            return !(*this == other);
        }

        iterator begin() {
            return Iterator(*this, mostLeft(), false);
        }

        iterator end() {
            return Iterator(*this, nullptr, true);
        }

        const_iterator cbegin() const {
            return ConstIterator(*this, mostLeft(), false);
        }

        const_iterator cend() const {
            return ConstIterator(*this, nullptr, true);
        }

        const_iterator begin() const {
            return cbegin();
        }

        const_iterator end() const {
            return cend();
        }

    private:
        TreeNode* mRoot; // wskaźnik na korzeń drzewa
        size_type mCount; // ilość węzłów drzewa
        //Wstawianie elementu do drzewa:
        TreeNode* insert(value_type pValue) {
            TreeNode* node = allocate(pValue.first);
            node->mPair.second = pValue.second;
            return node;
        };
        //Funkcja odszukuje miejsce, gdzie ma być wstawiony węzeł i go wstawia po wartości klucza:
        TreeNode* allocate(key_type pKey) {
            TreeNode* new_node = new TreeNode(std::make_pair(pKey, ValueType()));
            TreeNode* node = mRoot; // zaczynamy od roota
            TreeNode* parent; //wskazanie na rodzica ( schodzimy w dół)
            ++mCount;
            //Jeżeli drzewo jest puste:
            if (mRoot == nullptr) {
                mRoot = new_node;
                return new_node;
            }

            while (true) {
                parent = node;
                // Schodzimy w lewo (wartości klucza mniejsze od parent):
                if (node->mPair.first > pKey) {
                    node = node->mLeft;
                    if (node == nullptr) {
                        new_node->mParent = parent;
                        parent->mLeft = new_node;
                        break;
                    }
                } else {//schodzimy w prawo:
                    node = node->mRight;
                    if (node == nullptr) {
                        new_node->mParent = parent;
                        parent->mRight = new_node;
                        break;
                    }
                }
            }
            rebalance(parent);
            return new_node;
        };

        void removeNode(KeyType pKey) {
            if (mRoot == nullptr)
                throw std::out_of_range("Node not found.");

            TreeNode* n = mRoot;
            TreeNode* parent = mRoot;
            TreeNode* child = mRoot;
            TreeNode* delNode = nullptr;

            while (child != nullptr) {
                parent = n;
                n = child;
                if (pKey >= n->mPair.first)
                    child = n->mRight;
                else
                    child = n->mLeft;

                if (pKey == n->mPair.first)
                    delNode = n;
            }
            //Jeżeli nie znaleziono:
            if (!delNode)
                throw std::out_of_range("Node not found.");
            //Znaleziony:
            if (n->mLeft)
                child = n->mLeft;
            else
                child = n->mRight;
            //Jeżeli do usunięcia root:
            if (mRoot->mPair.first == pKey) {
                mRoot = child;
            } else {
                if (parent->mLeft == n)
                    parent->mLeft = child;
                else
                    parent->mRight = child;
                rebalance(parent);//wyrównanie drzewa
            }
            delete n;
            --mCount;
        }


        TreeNode* findNode(const key_type& pKey) const {
            TreeNode* root = mRoot;
            while (root != nullptr && root->mPair.first != pKey)
                if (root->mPair.first < pKey)
                    root = root->mRight;
                else
                    root = root->mLeft;
            return root;
        }

        void clear(TreeNode* pRoot) {

            if (pRoot == nullptr)
                return;
            //Rekurencyjne wywołanie (przejście po całym drzewie):
            clear(pRoot->mLeft);
            clear(pRoot->mRight);

            if (pRoot->mLeft != nullptr) {
                --mCount;
                delete pRoot->mLeft;
                pRoot->mLeft = nullptr;
            }
            if (pRoot->mRight != nullptr) {
                --mCount;
                delete pRoot->mRight;
                pRoot->mRight = nullptr;
            }
            if (pRoot == mRoot) {
                delete mRoot;
                mRoot = nullptr;
                --mCount;
            }
        }
        //Zwracanie node'a z najmniejszym kluczem:
        TreeNode* mostLeft() const {
            TreeNode* temp = mRoot;
            if (temp == nullptr)
                return nullptr;
            while (temp->mLeft != nullptr)
                temp = temp->mLeft;
            return temp;
        }
        //Największy klucz:
        TreeNode* mostRight() const {
            TreeNode* temp = mRoot;
            if (temp == nullptr)
                return nullptr;

            while (temp->mRight)
                temp = temp->mRight;
            return temp;
        }
        //Funkcja wyrównująca drzewo:
        void rebalance(TreeNode* pRoot) {

            pRoot->mHeight = 1 + std::max(getHeight(pRoot->mLeft), getHeight(pRoot->mRight));

            int balance = getHeight(pRoot->mRight) - getHeight(pRoot->mLeft);
            //Gdy wysokość większa po stronie lewej:
            if (balance == -2) {
                if (getHeight(pRoot->mLeft->mRight) - getHeight(pRoot->mLeft->mLeft) > 0)
                    pRoot->mLeft = rotateLeft(pRoot->mLeft);
                rotateRight(pRoot);
            //Po stronie prawej
            } else if (balance == 2) {
                if (getHeight(pRoot->mRight->mRight) - getHeight(pRoot->mRight->mLeft) < 0)
                    pRoot->mRight = rotateRight(pRoot->mRight);
                rotateLeft(pRoot);
            }

            if (pRoot->mParent != nullptr)
                rebalance(pRoot->mParent);
            else
                mRoot = pRoot;
        }
        //Obrót w lewo
        TreeNode* rotateLeft(TreeNode* pRoot) {
            TreeNode* temp = pRoot->mRight;
            temp->mParent = pRoot->mParent;
            pRoot->mRight = temp->mLeft;

            if (pRoot->mRight != nullptr)
                pRoot->mRight->mParent = pRoot;

            temp->mLeft = pRoot;
            pRoot->mParent = temp;

            if (temp->mParent != nullptr) {
                if (temp->mParent->mLeft == pRoot)
                    temp->mParent->mLeft = temp;
                else
                    temp->mParent->mRight = temp;
            }
            pRoot->mHeight = std::max(getHeight(pRoot->mLeft), getHeight(pRoot->mRight)) + 1;
            temp->mHeight = std::max(getHeight(temp->mLeft), getHeight(temp->mRight)) + 1;

            return temp;
        }

        //Obrót w prawo:
        TreeNode* rotateRight(TreeNode* pRoot) {
            TreeNode* temp = pRoot->mLeft;
            temp->mParent = pRoot->mParent;
            pRoot->mLeft = temp->mRight;

            if (pRoot->mLeft != nullptr)
                pRoot->mLeft->mParent = pRoot;

            temp->mRight = pRoot;
            pRoot->mParent = temp;

            if (temp->mParent != nullptr) {
                if (temp->mParent->mRight == pRoot)
                    temp->mParent->mRight = temp;
                else
                    temp->mParent->mLeft = temp;
            }
            pRoot->mHeight = std::max(getHeight(pRoot->mLeft), getHeight(pRoot->mRight)) + 1;
            temp->mHeight = std::max(getHeight(temp->mLeft), getHeight(temp->mRight)) + 1;

            return temp;
        }

        inline int getHeight(const TreeNode* pRoot) const {
            if (pRoot == nullptr)
                return -1;
            return pRoot->mHeight;
        }
    };
    //Węzeł drzewa:
    template<typename KeyType, typename ValueType>
    struct TreeMap<KeyType, ValueType>::TreeNode {
        value_type mPair;//para klucz porządkujący/wartość
        TreeNode* mParent;//wskaźnik na rodzica (dla root nullptr)
        TreeNode* mLeft;//wskaźnik na lewego potomka
        TreeNode* mRight;//wskaźnik na prawego potomka
        int mHeight;//wysokość węzła

        TreeNode() : mPair(std::make_pair(KeyType(), ValueType())), mParent(nullptr), mLeft(nullptr), mRight(nullptr),
                     mHeight(0) {}

        TreeNode(value_type pPair) : mPair(pPair), mParent(nullptr), mLeft(nullptr), mRight(nullptr), mHeight(0) {}
    };

    template<typename KeyType, typename ValueType>
    class TreeMap<KeyType, ValueType>::ConstIterator {
    public:
        using reference = typename TreeMap::const_reference;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename TreeMap::value_type;
        using pointer = const typename TreeMap::value_type*;

        friend class TreeMap;

        explicit ConstIterator(const TreeMap& pMap, TreeNode* pNode, bool pEnd) : mMap(pMap), mNode(pNode),
                                                                                  mEnd(pEnd) {}

        ConstIterator(const ConstIterator& other) : ConstIterator(other.mMap, other.mNode, other.mEnd) {}

        ConstIterator& operator++() {
            if (mNode == nullptr)
                throw std::out_of_range("End of tree.");
            //Jeżeli istnieje prawy potomek, to w prawo i do końca w lewo:
            if (mNode->mRight != nullptr) {
                mNode = mNode->mRight;
                while (mNode->mLeft) mNode = mNode->mLeft;
            } else {//jeżeli nie to do parenta:
                while (mNode->mParent != nullptr && mNode->mParent->mRight == mNode)
                    mNode = mNode->mParent;
                mNode = mNode->mParent;
                if (mNode == nullptr)
                    mEnd = true;
            }
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator temp(*this);
            operator++();
            return temp;
        }

        ConstIterator& operator--() {
            if (mNode == nullptr) {
                if (!mEnd) {
                    throw std::out_of_range("At the beginning.");
                } else {
                    mNode = mMap.mostRight();
                    return *this;
                }
            }
            if (mNode->mLeft) {
                mNode = mNode->mRight;
                while (mNode->mRight) mNode = mNode->mRight;
            } else {
                while (mNode->mParent && mNode->mParent->mLeft == mNode) mNode = mNode->mParent;
                mNode = mNode->mParent;
            }
            return *this;
        }

        ConstIterator operator--(int) {
            ConstIterator temp(*this);
            operator--();
            return temp;
        }

        reference operator*() const {
            if (mNode == nullptr)
                throw std::out_of_range("Dereferencing used iterator");
            return mNode->mPair;
        }

        pointer operator->() const {
            return &this->operator*();
        }

        bool operator==(const ConstIterator& other) const {
            return mNode == other.mNode;
        }

        bool operator!=(const ConstIterator& other) const {
            return !(*this == other);
        }

    private:
        const TreeMap& mMap; // wskaźnik na drzewo
        TreeNode* mNode; // aktualnie wskazywany węzeł
        bool mEnd; // czy końcowy?
    };

    template<typename KeyType, typename ValueType>
    class TreeMap<KeyType, ValueType>::Iterator : public TreeMap<KeyType, ValueType>::ConstIterator {
    public:
        using reference = typename TreeMap::reference;
        using pointer = typename TreeMap::value_type*;

        explicit Iterator(const TreeMap& pMap, TreeNode* pNode, bool pEnd) : ConstIterator(pMap, pNode, pEnd) {}

        Iterator(const ConstIterator& other)
                : ConstIterator(other) {}

        Iterator& operator++() {
            ConstIterator::operator++();
            return *this;
        }

        Iterator operator++(int) {
            auto result = *this;
            ConstIterator::operator++();
            return result;
        }

        Iterator& operator--() {
            ConstIterator::operator--();
            return *this;
        }

        Iterator operator--(int) {
            auto result = *this;
            ConstIterator::operator--();
            return result;
        }

        pointer operator->() const {
            return &this->operator*();
        }

        reference operator*() const {
            // ugly cast, yet reduces code duplication.
            return const_cast<reference>(ConstIterator::operator*());
        }
    };

}

#endif /* AISDI_MAPS_MAP_H */
