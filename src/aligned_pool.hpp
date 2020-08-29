#pragma once

namespace puro {

/** Memory aligned pool based on a vector. To avoid cache misses, elements are stored in a consecutive order,
    and moved around upon popping. Works best for Elements with small size and trivial copy operation.
*/
template <typename T>
struct AlignedPool
{
    struct Iterator
    {
        Iterator(AlignedPool<T>& p, int i) : pool(p), index(i) {}
        AlignedPool<T>& pool;
        int index;

        T& get() noexcept { return pool.elements[index]; }

        bool is_valid() const noexcept { return index >= 0; }

        bool operator!= (const Iterator& other) noexcept { return index != other.index; }
        Iterator& operator*() noexcept { return *this; }
        T* operator->() noexcept { return &pool[index]; }
        Iterator& operator++() noexcept { --index; return *this; }
    };

    Iterator push(T&& element) noexcept
    {
        if (elements.size() < elements.capacity())
        {
            elements.push_back(element);
            return Iterator(*this, (int)size() - 1);
        }

        return Iterator(*this, -1);
    }

    void pop(const Iterator& it) noexcept
    {
        pop(it.index);
    }
    
    void pop(int index) noexcept
    {
        if ((int)size() > 1 && index < (int)size() - 1)
        {
            memcpy(&elements[index], &elements[size() - 1], sizeof(T));
        }
        
        elements.pop_back();
    }

    Iterator begin() noexcept { return Iterator(*this, (int)size() - 1); }
    Iterator end() noexcept { return Iterator(*this, -1); }

    size_t size() const noexcept { return elements.size(); }

    std::vector<T> elements;
};

}
