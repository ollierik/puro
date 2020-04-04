
#include <iostream>
#include <memory>


struct Element
{
    Element() : x(0), y(0) {}
    Element(int x, int y) : x(x), y(y) {}

    void set(int i)
    {
        x = i;
        y = -i;
    }

    void print()
    {
        std::cout << x << ", " << y << std::endl;
    }

    int x, y;
};

int main()
{
    const int n = 8;
    Element elements[n];

    for (int i=0; i<n; i++)
        elements[i] = { i, -i };

    {
        Element* ptr = &elements[2];
        new (ptr) Element(100, -100);
    }

    Element* mem = &elements[4];
    std::unique_ptr<Element, void> ptr = std::unique_ptr<Element>(new (mem) Element());
    ptr->set(1000);

    for (int i=0; i<n; i++)
        elements[i].print();


    return 0;
}
