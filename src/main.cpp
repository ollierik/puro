#include <iostream>
#include <vector>
#include "Pool.hpp"

class Element
{
public:
    Element(int i)
    {
        x = (size_t)i;
    }
    Element() = delete;

    void print()
    {
        std::cout << "Element value:" << x << std::endl;
    }

    size_t x;
};

int main ()
{
    const auto n = 10;
    Pool<Element, n> pool;
    std::cout << sizeof(Element) << std::endl;
    std::cout << sizeof(pool) << std::endl;
    std::cout << pool.size() << std::endl;

    for (int i=0; i<4; i++)
    {
        Element* e = pool.add(i*10);
    }

    int c = 0;
    for (auto e : pool)
    {
        e.print();
        std::cout << "c:" << c << std::endl;
        ++c;
    }


    return 0;
}
