#include <iostream>
#include <vector>
#include "Pool.hpp"

class Element
{
public:
    Element(int i)
    {
        x = i;
        y = i;
    }
    //Element() = delete;
    Element() = default;

    void print()
    {
        std::cout << "Element value: " << x << std::endl;
    }

    int x;
    int y;
};

int main ()
{
    const auto n = 10;
    PoolMemory<Element, 8> mem;

    for (int i=0; i<n; i++)
    {
        auto e = mem.add(i*10);
        if (e != nullptr)
            e->print();
        else
            std::cout << "Couldn't add, container full" << std::endl;
    }

    std::cout << "Print all elements" << std::endl;
    for (int i = 0; i < mem.size(); i++)
    {
        mem[i].print();
    }

    {
        Element* elementToRelease = &mem[2];
        std::cout << "Element to release at location 1:\n    ";
        elementToRelease->print();
        std::cout << std::endl;
        mem.release(elementToRelease);
    }

    std::cout << "Print all elements" << std::endl;
    for (int i = 0; i < mem.size(); i++)
    {
        mem[i].print();
    }

    {
        Element* elementToRelease = &mem[2];
        std::cout << "Element to release at location 1:\n    ";
        elementToRelease->print();
        std::cout << std::endl;
        mem.release(elementToRelease);
    }

    std::cout << "Print all elements" << std::endl;
    for (int i = 0; i < mem.size(); i++)
    {
        mem[i].print();
    }

    /*
    FixedPool<Element, n> pool;
    //DynamicPool<Element, n> pool;
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
    */
}
