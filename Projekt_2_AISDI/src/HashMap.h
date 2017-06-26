#ifndef AISDI_MAPS_HASHMAP_H
#define AISDI_MAPS_HASHMAP_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <functional>//std::hash dla typów wbudowanych
#include <iostream>

namespace aisdi
{

template <typename KeyType, typename ValueType>
class HashMap
{
public:
    using key_type = KeyType;
    using mapped_type = ValueType;
    using value_type = std::pair<const key_type, mapped_type>;
    using size_type = std::size_t;
    using reference = value_type&;
    using const_reference = const value_type&;

    class ConstIterator;
    class Iterator;
    class BucketNode;
    using iterator = Iterator;
    using const_iterator = ConstIterator;
    //Konstruktor przyjmuje liczbę "wiaderek", tworzy tablicę wskaźników na węzły (listę węzłów)
    HashMap(size_type Buckets = 50): mBucketCount(Buckets), mCount(0)
    {
        mBuckets = new BucketNode*[mBucketCount];
        for(size_type i = 0; i<mBucketCount; ++i)
            mBuckets[i] = nullptr;
        mHasher = [](const key_type& pKey) {
            return std::hash<key_type>{}(pKey);
        };
    }

    HashMap(std::initializer_list<value_type> list):HashMap()
    {
        for(auto&& item : list)//pojedynczy node jako wskaźnik podwójny (dlatego &&)
            insert(item.first, item.second);

    }

    HashMap(const HashMap& other):HashMap()
    {
        mHasher = other.mHasher;
        for(auto it = other.cbegin(); it != other.cend(); ++it)
            insert((*it).first, (*it).second);
    }

    HashMap(HashMap&& other):HashMap()
    {
        std::swap(mCount, other.mCount);
        std::swap(mBuckets, other.mBuckets);
        std::swap(mBucketCount, other.mBucketCount);
        std::swap(mHasher, other.mHasher);
    }

    HashMap& operator=(const HashMap& other)
    {
        if(*this == other)
            return *this;
        clear();
        for(auto&& item : other)
        {
            insert(item.first, item.second);
        }
        return *this;
    }

    HashMap& operator=(HashMap&& other)
    {
        std::swap(mCount, other.mCount);
        std::swap(mBuckets, other.mBuckets);
        std::swap(mBucketCount, other.mBucketCount);
        std::swap(mHasher, other.mHasher);
        other.mCount = 0;
        return *this;
    }

    bool isEmpty() const
    {
        return mCount == 0;
    }
    //Wyszukanie po indeksie (kluczu):
    mapped_type& operator[](const key_type& key)
    {
        size_type tempBucket = bucketHash(key);
        BucketNode* tempNode = mBuckets[tempBucket];//Wyszukanie początkowego node'a w odpowiednim wiaderku
        while(tempNode != nullptr && tempNode->mPair.first != key)
            tempNode = tempNode->mNextNode;
        if (tempNode == nullptr)
            return (*insert(key)).second;
        else
            return tempNode->mPair.second;
    }
    //Zwraca wartość elementu o danym kluczu:
    const mapped_type& valueOf(const key_type& key) const
    {
        auto it = find(key);//zwraca iterator na element o danym kluczu
        if (it != end())
            return (*it).second;
        else
            throw std::out_of_range("Key not found.");
    }
    //Rzutowanie w celu zmniejszenia objętości kodu:
    mapped_type& valueOf(const key_type& key)
    {
        return const_cast<mapped_type&>(static_cast<const HashMap<KeyType, ValueType>*>(this)->valueOf(key));
    }

    const_iterator find(const key_type& key) const
    {
        size_type bucket = bucketHash(key);
        BucketNode* temp = mBuckets[bucket];
        while (temp != nullptr && temp->mPair.first != key)
            temp = temp->mNextNode;
        if (temp == nullptr)
            return end();
        return ConstIterator(*this, bucket, temp);
    }
    //Rzutowanie w celu zmniejszenia objętości kodu:
    iterator find(const key_type& key)
    {
        return static_cast<const HashMap<KeyType, ValueType>*>(this)->find(key);
    }
    //Usuwanie elementu:
    void remove(const key_type& key)
    {
        size_type bucket = bucketHash(key);
        BucketNode* temp = mBuckets[bucket];
        //Jeżeli nie ma takiego wiaderka:
        if (temp == nullptr)
            throw std::out_of_range("Key not found.");

        if (temp->mPair.first == key) // w przypadku gdy jest to element pierwszy w danym wiaderku
        {
            mBuckets[bucket] = temp->mNextNode;
            delete temp;
            --mCount;
            return;
        }
        //Przeszukiwanie wiaderka:
        while (temp != nullptr && temp->mNextNode != nullptr && temp->mNextNode->mPair.first != key)
            temp = temp->mNextNode;

        if (temp == nullptr)
            throw std::out_of_range("Key not found");
        else{
        temp->mNextNode = (temp->mNextNode)->mNextNode;
        --mCount;
        delete temp;
        }
    }

    void remove(const const_iterator& it)
    {
        if (it == end())
            throw std::out_of_range("Trying to remove end.");

        BucketNode* temp;
        temp = mBuckets[it.mBucket];

        if (temp == it.mNode)//jeżeli pierwszy
        {
            mBuckets[it.mBucket] = temp->mNextNode;
            delete temp;
            --mCount;
            return;
        }

        while (temp->mNextNode != it.mNode)
            temp = temp->mNextNode;
        temp->mNextNode = temp->mNextNode->mNextNode;
        --mCount;
        delete temp;
    }

    size_type getSize() const
    {
        return mCount;
    }

    bool operator==(const HashMap& other) const
    {
        if (mCount != other.mCount)
            return false;

        for (size_type i = 0; i < mBucketCount; ++i)
        {   //Czy mają wiaderka o danym indeksie:
            if ((mBuckets[i] == nullptr && other.mBuckets[i] != nullptr) ||
                    (mBuckets[i] != nullptr && other.mBuckets[i] == nullptr))
                return false;

            //Sprawdzanie w przypadku gdy są elementy:
            BucketNode* node = mBuckets[i];
            while (node != nullptr)
            {
                if (other.find(node->mPair.first)->second != node->mPair.second)
                    return false;
                node = node->mNextNode;
            }
        }

        return true;
    }

    bool operator!=(const HashMap& other) const
    {
        return !(*this == other);
    }

    iterator begin()
    {
        return Iterator(*this, 0, mBuckets[0]);
    }

    iterator end()
    {
        return Iterator(*this, mBucketCount - 1, nullptr);
    }

    const_iterator cbegin() const
    {
        return ConstIterator(*this, 0, mBuckets[0]);
    }

    const_iterator cend() const
    {
        return ConstIterator(*this, mBucketCount - 1, nullptr);
    }

    const_iterator begin() const
    {
        return cbegin();
    }

    const_iterator end() const
    {
        return cend();
    }

private:
    size_type mBucketCount;//liczba wiaderek
    size_type mCount;// liczba wszystkich węzłów
    BucketNode** mBuckets;
    //Funkcja hashująca (standardowa):
    std::function<size_type(const key_type&)> mHasher;
    //Zwraca indeks wiaderka:
    size_type bucketHash(const key_type& pKey) const
    {
        return mHasher(pKey) % mBucketCount;
    }

    size_type hash(const key_type& pKey) const
    {
        return mHasher(pKey);
    }

    iterator insert(const key_type& pKey, mapped_type pValue)
    {
        size_type bucket = bucketHash(pKey);//które wiaderko
        BucketNode* temp = new BucketNode(pKey, pValue);
        if (mBuckets[bucket] != nullptr)
            temp->mNextNode = mBuckets[bucket];
        mBuckets[bucket] = temp;//wstawianie na początek wiaderka
        ++mCount;
        return Iterator(*this, bucket, temp);
    };

    iterator insert(const key_type& pKey)
    {
        size_type bucket = bucketHash(pKey);
        BucketNode* temp = new BucketNode(pKey);
        if (mBuckets[bucket] != nullptr)
            temp->mNextNode = mBuckets[bucket];
        mBuckets[bucket] = temp;
        ++mCount;
        return Iterator(*this, bucket, temp);
    };
    //Usuwanie wszystkich rekordów:
    void clear()
    {
        BucketNode* node;
        BucketNode* temp;

        for (size_type i = 0; i < mBucketCount; ++i)
        {
            node = mBuckets[i];
            while (node != nullptr)
            {
                temp = node;
                node = node->mNextNode;
                delete temp;
                --mCount;
            }
            mBuckets[i] = nullptr;
        }
    };
};

//Pojedynczy "węzeł" hashmapy:
template<typename KeyType, typename ValueType>
struct HashMap<KeyType, ValueType>::BucketNode
{   //Para klucz/wartość:
    value_type mPair;
    //Wskaźnik na następny węzeł w wiaderku:
    BucketNode* mNextNode;
    //Konstruktory:
    BucketNode(const key_type& pKey) : mPair(std::make_pair(pKey, ValueType {})), mNextNode(nullptr) {}
    BucketNode(const key_type& pKey, mapped_type pData) : mPair(std::make_pair(pKey, pData)), mNextNode(nullptr) {}
    BucketNode(value_type pPair) : mPair(pPair), mNextNode(nullptr) {}
};

template <typename KeyType, typename ValueType>
class HashMap<KeyType, ValueType>::ConstIterator
{
public:
    using reference = typename HashMap::const_reference;
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename HashMap::value_type;
    using pointer = const typename HashMap::value_type*;

    friend class HashMap;

    explicit ConstIterator(const HashMap<KeyType, ValueType>& Map, size_type Bucket, BucketNode* Node) : mMap(
            Map), mBucket(Bucket), mNode(Node)
    {
            while (mNode == nullptr && mBucket < mMap.mBucketCount - 1)
        {
            ++mBucket;
            mNode = mMap.mBuckets[mBucket];
        }
    }

    ConstIterator(const ConstIterator& other) : mMap(other.mMap), mBucket(other.mBucket), mNode(other.mNode) {}

    ConstIterator& operator++()
    {
        if (mNode == nullptr)
            throw std::out_of_range("Cannot increment end.");

        mNode = mNode->mNextNode;
        while (mNode == nullptr && mBucket < mMap.mBucketCount - 1)
        {
            ++mBucket;
            mNode = mMap.mBuckets[mBucket];
        }

        return *this;
    }

    ConstIterator operator++(int)
    {
        ConstIterator temp(*this);
        operator++();
        return temp;
    }

    ConstIterator& operator--()
    {
        if (mMap.mBuckets[mBucket] == mNode)
        {
            while (mMap.mBuckets[mBucket] == nullptr && mBucket > 0) mBucket--;
            mNode = mMap.mBuckets[mBucket];

            if (mBucket < 0 || mNode == nullptr)
                throw std::out_of_range("Cannot decrement beginning.");

            while (mNode->mNextNode != nullptr) mNode = mNode->mNextNode;

        }
        else
        {
            BucketNode* node = mMap.mBuckets[mBucket];
            while (node->mNextNode != mNode) node = node->mNextNode;
            mNode = node;
        }
        return *this;
    }

    ConstIterator operator--(int)
    {
        ConstIterator temp(*this);
        operator--();
        return temp;
    }

    reference operator*() const
    {
        if (mNode == nullptr)
            throw std::out_of_range("Cannot dereference nullptr.");
        return mNode->mPair;
    }

    pointer operator->() const
    {
        return &this->operator*();
    }

    bool operator==(const ConstIterator& other) const
    {
        return (mNode == other.mNode && mBucket == other.mBucket);
    }

    bool operator!=(const ConstIterator& other) const
    {
        return !(*this == other);
    }

private:
    const HashMap& mMap;
    size_type mBucket;
    BucketNode* mNode;
};

template <typename KeyType, typename ValueType>
class HashMap<KeyType, ValueType>::Iterator : public HashMap<KeyType, ValueType>::ConstIterator
{
public:
    using reference = typename HashMap::reference;
    using pointer = typename HashMap::value_type*;

    explicit Iterator(const HashMap<KeyType, ValueType>& Map, size_type Bucket, BucketNode* Node)
        : ConstIterator(Map, Bucket, Node) {}

    Iterator(const ConstIterator& other)
        : ConstIterator(other)
    {}

    Iterator& operator++()
    {
        ConstIterator::operator++();
        return *this;
    }

    Iterator operator++(int)
    {
        auto result = *this;
        ConstIterator::operator++();
        return result;
    }

    Iterator& operator--()
    {
        ConstIterator::operator--();
        return *this;
    }

    Iterator operator--(int)
    {
        auto result = *this;
        ConstIterator::operator--();
        return result;
    }

    pointer operator->() const
    {
        return &this->operator*();
    }

    reference operator*() const
    {
        // ugly cast, yet reduces code duplication.
        return const_cast<reference>(ConstIterator::operator*());
    }
};

}

#endif /* AISDI_MAPS_HASHMAP_H */
