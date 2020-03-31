#include <iostream>

template <typename FloatType, int N>
class Engine
{
public:
    void foo() { std::cout << "primary foo\n"; }
    void bar() { std::cout << "primary bar\n"; }
};

template <class FloatType>
class Engine<FloatType, 1>
{
public:
    void foo() { std::cout << "special foo\n"; }
};

int main()
{
    Engine<float, 0> e0;
    Engine<float, 1> e1;

    e0.foo();
    e0.bar();
    e1.foo();
    e1.bar();
    return 0;
}
