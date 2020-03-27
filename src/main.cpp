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
    //FixedPool<Element, 8> pool;
    DynamicPool<Element, 4> pool;

    for (auto i=0; i<6; i++)
    {
        Element* e = pool.add((size_t)(i*10));
    }

    std::cout << "Print all Elements: " << std::endl;
    for (auto& e : pool)
    {
        e->print();
    }

    std::cout << "Remove element while iterating: " << std::endl;
    for (auto& e : pool)
    {
        if (e->x == 20)
        {
            std::cout << "*** Remove element *** ";
            e->print();
            pool.remove(e);
        }
        else
        {
            e->print();
        }
    }

    std::cout << "Print all Elements: " << std::endl;
    for (auto& e : pool)
    {
        e->print();
    }

    return 0;
}
