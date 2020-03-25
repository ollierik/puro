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
    //std::cout << sizeof(Pool<Element, n>::EndMarker) << std::endl;

    for (auto i=0; i<4; i++)
    {
        Element* e = pool.add((size_t)(i*10));
    }

    int c = 0;
    for (auto it : pool)
    {
        //std::cout << "c: " << c << std::endl;


        if (c == 1)
            pool.remove(it);
        ++c;
    }


    return 0;
}
