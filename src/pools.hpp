#pragma once

/** Memory aligned pool based on a vector. To avoid cache misses, elements are stored in a consecutive order,
    and moved around upon popping. Works best for Elements with small size and trivial copy operation.
*/
template <typename T>
struct AlignedPool
{
    struct Iterator
    {
        Iterator(AlignedPool<T>& p, int i) : pool(p), index(i) {}
        int index;
        AlignedPool<T>& pool;

        T& get() { return pool.elements[index]; }

        bool isValid() { return index >= 0; }

        bool operator!= (const Iterator& other) { return index != other.index; }
        Iterator& operator*() { return *this; }
        T* operator->() { return &pool[index]; }
        Iterator& operator++() { --index; return *this; }
    };


    Iterator push(T&& element)
    {
        if (elements.size() < elements.capacity())
        {
            elements.push_back(element);
            return Iterator(*this, (int)size()-1);
        }

        return Iterator(*this, -1);
    }

    void pop(const Iterator& it)
    {
        if ((int)size() > 1 && it.index < (int)size()-1)
        {
            memcpy(&elements[it.index], &elements[size()-1], sizeof(T));
        }

        elements.pop_back();
    }

    Iterator begin() { return Iterator(*this, (int)size()-1); }
    Iterator end() { return Iterator(*this, -1); }

    size_t size() { return elements.size(); }

    std::vector<T> elements;
};


