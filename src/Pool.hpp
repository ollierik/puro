#include <vector>
#include <memory>

template <class ElementType>
class Pool
{
public:

    //int size() const { return static_cast<int>(vec.size()); }
    virtual int size() const = 0;
    virtual const ElementType* data() const = 0;
    //ElementType& operator [] (int i) { return *(vec[i]); }
    virtual ElementType& operator [] (int i);

    /**
    template<typename... Args>
    bool emplace(Args... args)
    {
        if (capacity() != size())
        {
            auto e = std::make_unique<ElementType> (args);
            vec.push_back(e);
            return true;
        }
        return false;
    }
     */

    
    /*
    bool add(ElementPtr e)
    {
        if (capacity() != size())
        {
            vec.push_back(std::move(e));
            return true;
        }
        return false;
    }

    void remove(int index)
    {
        const auto numElements = vec.size();
        const auto indexOfLast = numElements - 1;
        
        if (numElements > 1)
        {
            vec[index].swap(vec[indexOfLast]);
        }
        
        vec.erase(vec.begin() + indexOfLast);
    }
    */

    /** Returns negative if didn't succeed */
    template<typename... Args>
    virtual int emplace(Args... args) = 0;

    virtual = void remove(int index) = 0;
};

template <class ElementType>
class Pool
{
private:

    using ElementPtr = std::unique_ptr<ElementType>;

    std::vector<ElementPtr> vec;

public:

    //Pool() = default;
    /**
     How many elements to hold
     */
    Pool(int capacity)
    {
        vec.reserve(capacity);
    }

    void reserve(int size) { vec.reserve(size); }
    int capacity() const { return vec.capacity(); }
    int size() const { return static_cast<int>(vec.size()); }
    void clear() { vec.clear(); }
    std::vector<ElementPtr>& vector() { return vec; }
    ElementType* data() { return vec.data(); }

    ElementType& operator [] (int i) { return *(vec[i]); }

    /**
     Adds new element by emplacing it back.

     @param args Arguments that will be passed to the constructor of the ElementType
     @return true if success, false if no more Elements can fit the container
    template<typename... Args>
    bool emplace(Args... args)
    {
        if (capacity() != size())
        {
            auto e = std::make_unique<ElementType> (args);
            vec.push_back(e);
            return true;
        }
        return false;
    }
     */

    
    /**
     Adds a new element by moving.

     @param Pass the element as a std::unique_ptr with std::move(xyz)
     @return true if success, false if no more Elements can fit the container
     */
    bool add(ElementPtr e)
    {
        if (capacity() != size())
        {
            vec.push_back(std::move(e));
            return true;
        }
        return false;
    }

    void remove(int index)
    {
        const auto numElements = vec.size();
        const auto indexOfLast = numElements - 1;
        
        if (numElements > 1)
        {
            vec[index].swap(vec[indexOfLast]);
        }
        
        vec.erase(vec.begin() + indexOfLast);
    }
};
