#include <iostream>
#include <cstdlib>
#include <memory>

struct ElementPart
{
    ElementPart(float val) : f(val) {}
    float f;
};

struct Element
{
    Element() = delete;
    Element(int x, ElementPart ep) : x(x), part(ep) {}

    void set(int i)
    {
        x = i;
    }

    void print()
    {
        std::cout << x << ", " << part.f << std::endl;
    }

    int x;
    ElementPart part;
};


template <class ElementType>
struct Pool
{
    Pool() = default;

    template <typename... Args>
    ElementType* emplace(Args... args)
    {
        const float r = ((float)std::rand()) / ((float)RAND_MAX);
        std::cout << "rand: " <<  r << std::endl;
        if (r > 0.5f)
        {
            new (element) ElementType(args...);
            return reinterpret_cast<ElementType*> (element);
        }

        return nullptr;
    }

    void* element[sizeof(ElementType)];

};

int main()
{
    std::srand(5);
    Pool<Element> pool;

    for (int i = 0; i < 10; i++)
    {
        Element* e = pool.emplace(3, ElementPart(2.0f));
        if (e != nullptr)
        {
            e->print();
        }
        else
        {
            std::cout << "nullptr\n";
        }
    }

    return 0;
}
