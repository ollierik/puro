#pragma once

#include "puro.hpp"

struct Grain 
{
    Grain(int val) : x(val) {}
    int x;
};


int main()
{
    puro::StackMemoryAllocator<Grain> stackMemory;
    puro::SafeStack<Grain> inactives;
    puro::NodeStack<Grain> actives;
    puro::NodeStack<Grain> removed;

    stackMemory.allocateChunk(4, inactives);
    stackMemory.allocateChunk(3, inactives);
    stackMemory.allocateChunk(2, inactives);
    stackMemory.allocateChunk(1, inactives);

    std::cout << "Create elements" << std::endl;
    for (int i=0; !inactives.empty(); ++i)
    {
        auto* it = inactives.pop_front();

        if (it != nullptr)
            it->getElement() = Grain(i);

        std::cout << it->getElement().x << std::endl;
        actives.push_front(it);
    }

    std::cout << "Iterate elements" << std::endl;
    for (auto&& it : actives)
    {
        std::cout << it.get().x << std::endl;
    }

    std::cout << "Iterate elements, remove if x % 2 == 0" << std::endl;
    for (auto&& it : actives)
    {
        std::cout << it.get().x << std::endl;

        if (it.get().x % 2 == 0)
        {
            auto popped = actives.pop(it);
            removed.push_front(popped);
        }
    }

    std::cout << "Iterate actives" << std::endl;
    for (auto&& it : actives)
    {
        std::cout << it.get().x << std::endl;
    }

    std::cout << "Iterate removed" << std::endl;
    for (auto&& it : removed)
    {
        std::cout << it.get().x << std::endl;
    }

    inactives.push_multiple(removed.pop_all());

    std::cout << "Iterate inactives after removed -> inactives" << std::endl;
    puro::Node<Grain>* node = inactives.first();
    while (node != nullptr)
    {
        std::cout << node->getElement().x << std::endl;
        node = node->next;
    }

    return 0;
}
