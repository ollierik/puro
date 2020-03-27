#include <iostream>
#include <vector>
#include "Pool.hpp"
#include "DynamicPool.hpp"

class Element
{
public:
    Element(int i)
    {
        x = i;
        y = i;
    }
    Element() = delete;

    void print()
    {
        std::cout << "    Element value: " << x << std::endl;
    }

    int x;
    int y;
};

int main ()
{
    const int n = 8;

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

    std::cout << "Print all Elements: " << std::endl;
    for (auto e : pool)
    {
        e->print();
    }

    std::cout << "Remove element while iterating: " << std::endl;
    for (auto e : pool)
    {
        if (e->x == 20)
        {
            std::cout << "*** Remove element" << std::endl;
            pool.remove(e);
        }
        else
        {
            e->print();
        }
    }

    std::cout << "Print all Elements: " << std::endl;
    for (auto e : pool)
    {
        e->print();
    }

    return 0;
}
