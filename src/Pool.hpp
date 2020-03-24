#include <array>
#include <vector>
#include <memory>

template <class ElementType>
class PoolInterface
{
public:

    /** Number of elements that can be stored into the pool with current memory allocation */
    virtual size_t capacity() const = 0;

    /** Number of currently stored (i.e. active) elements */
    virtual size_t size() const = 0;

    virtual ElementType& operator [] (int i) = 0;

    /** Creates new Element directly into the Pool.
       @return identifying index of the element that was created, or negative if couldn't add
    */
    template<typename... Args>
    virtual int add(Args... args) = 0;

    /** Removes the Element with the identifying index from the Pool. */
    virtual void remove(int index) = 0;
};




/** Pool with fixed amount of elements. Allocates elements to stack.
*/
template <class ElementType, int PoolSize>
class FixedPool : public PoolInterface<ElementType>
{
public:

    FixedPool()
    {
        // initialise lookup
        for (int i=0; i<capacity(); i++)
        {
            lookup[i] = &elements[i];
        }
    }

    constexpr int capacity() const { return PoolSize; }
    int size() const { return numActive; }
    ElementType& operator [] (int i) { return elements[i]; }

    template<typename... Args>
    int add(Args... args)
    {
        if (size() == capacity())
            return -1;

        // use first unused Element and increment
        const auto newElementIndex = lookup[numActive++];
        ElementType* element = &elements[newElementIndex]

        // construct in-place to avoid copying
        std::allocator<ElementType>::construct(element, args);

        return newElementIndex;
    }

    void remove(int index)
    {
        numActive -= 1;
        const auto indexOfLast = numActive;
        if (indexOfLast > 0)
        {
            swapLookups(index, indexOfLast);
        }
    }

protected:

    std::array<ElementType, PoolSize> elements;
    std::array<size_t, PoolSize> lookup;
    size_t numActive = 0;

private:

    void swapLookups(int i, int j)
    {
        auto* temp = lookup[i];
        lookup[i] = lookup[j];
        lookup[j] = temp;
    }
};




/** Pool with dynamic amount of elements.
*/
template <class ElementType, int InitialPoolSize, int MaxPoolSize = -1>
class DynamicPool : public PoolInterface<ElementType>
{
    DynamicPool()
    {
        // Reserve initial capacity
        elements.reserve(InitialPoolSize);
        lookup.reserve(InitialPoolSize);

        // Create lookup
        for (int i=0; i<InitialPoolSize; i++)
        {
            lookup.push_back(static_cast<size_t>(i));
        }

    }

    virtual size_t capacity() const { return elements.capacity(); };

    /** Number of currently stored (i.e. active) elements */
    virtual size_t size() const { return elements.size(); }

    virtual ElementType& operator [] (int i) { return elements[i]; }

    template<typename... Args>
    int add(Args... args)
    {
        if (size() == capacity())
        {
            if (! attemptToIncreaseCapacity())
                return -1;
        }

        // use first unused Element and increment
        const auto newElementIndex = lookup[numActive++];
        ElementType* element = &elements[newElementIndex]

        // construct in-place to avoid copying
        std::allocator<ElementType>::construct(element, args);

        return newElementIndex;
    }


protected:

    std::vector<ElementType> elements;
    std::vector<ElementType> lookup;

private:

    /** Returns true if capacity was incrased */
    bool attemptToIncreaseCapacity()
    {
        const auto currentCapacity = capacity();
        auto targetSize = capacity() * 2;
        if (targetSize > MaxPoolSize)
        {
            targetSize = MaxPoolSize;
        }
        elements.reserve(targetSize);

        return (capacity() > currentCapacity);
    }

};
