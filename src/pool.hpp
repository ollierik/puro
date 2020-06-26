#pragma once
#include <vector>

template <typename T>
struct Pool
{
    struct Iterator
    {
        Iterator(Pool<T>& p, int i) : pool(p), index(i) {}
        int index;
        Pool<T>& pool;

        T& get() { return pool.elements[index]; }

        bool operator!= (const Iterator& other) { return index != other.index; }
        Iterator& operator*() { return *this; }
        T* operator->() { return &pool[index]; }
        Iterator& operator++() { --index; return *this; }
    };


    T* push(T&& element)
    {
        if (elements.size() < elements.capacity())
        {
            elements.push_back(element);
            return &elements[size() - 1];
        }

        return nullptr;
    }

    void pop(const Iterator& it)
    {
        if (size() > 1 && it.index < size()-1)
        {
            memcpy(&elements[it.index], &elements[size()-1], sizeof(T));
            //elements[it.index] = memcpy((elements.back());
            elements.pop_back();
        }
    }

    Iterator begin() { return Iterator(*this, (int)size()-1); }
    Iterator end() { return Iterator(*this, -1); }

    size_t size() { return elements.size(); }

    std::vector<T> elements;
};


