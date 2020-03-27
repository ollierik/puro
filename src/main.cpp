#include <iostream>
#include <vector>
#include "Pool.hpp"

class Element
{
public:
    Element(size_t i)
    {
        x = i;
    }
    Element() = delete;

    void print()
    {
        std::cout << "Element value: " << x << std::endl;
    }

    bool terminated() const
    {
        return x == 20;
    }

    size_t x;
};

int main ()
{
    const auto n = 10;
    //FixedPool<Element, n> pool;
    DynamicPool<Element, n> pool;
    std::cout << "Size of Element: " << sizeof(Element) << std::endl;
    std::cout << "Size of pool: " << sizeof(pool) << std::endl;
    std::cout << "Pool size: " << pool.size() << std::endl;
    std::cout << "Pool capacity: " << pool.capacity() << std::endl;

    for (auto i=0; i<5; i++)
    {
        Element* e = pool.add((size_t)(i*10));
    }

    int i = 0;
    for (auto& e : pool)
    {
        std::cout << "\n# " << i << ":\n";

        if (i == 3)
        {
            pool.remove(e);
            e->x = -1;
        }
        else
        {
            e.print();
            e->print();
        }

        ++i;
    }

    return 0;
}
