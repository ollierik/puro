#pragma once

#include <iostream>
#include "Pool.hpp"

int poolMemoryTests()
{
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
            std::cout << "Element value: " << x << std::endl;
        }

        int x;
        int y;
    };

    const auto n = 10;
    PoolMemory<Element, 8> mem;

    for (int i = 0; i < n; i++)
    {
        auto e = mem.add(i * 10);
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
    return 0;
}

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
    Element() = delete;

    void print()
    {
        std::cout << "    Element value: " << x << std::endl;
    }

    int x;
    int y;
};

int poolTests()
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

