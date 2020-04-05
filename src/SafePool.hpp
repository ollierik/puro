#pragma once 

#include <array>

struct Element
{
    Element() = default;
    Element(int value) : n(value) {}

    int n = -1;
};


template <class ElementType>
class Pool
{
public:

    Pool()
    {
        for (int i=0; i<lookup.size(); ++i)
        {
            lookup[i] = &elements[i];
        }

        numUsed = 0;
        numStaged = 0;
    }

    void addElement(int value)
    {
        if (numUsed < n)
        {
            numUsed += 1; // atomic
            numStaged +=1 ;

            int addIndex = n - numStaged;
            ElementType* add = lookup[addIndex];

            *add = ElementType(value);
        }
    
    }

    int numUsed;
    int numAddition;

    constexpr int n = 8;

    std::array<ElementType*, n> lookup;
    std::array<ElementType, n> elements;
};
