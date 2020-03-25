#pragma once

#include <cstdlib>
#include <math.h>

class Interface
{
public:
    virtual int init(int y) = 0;
    virtual int operate(int x) = 0;
};

class Implementation : public Interface
{
public:
    virtual int init(int y) override
    {
        y = y;
    }

    virtual int operate(int x) override
    {
        return pow(x, y);
    }

    int y;
};

class Specialisation : public Implementation
{
public:
    virtual int init(int y) override
    {
        y = y*10;
    }

    int y;
};


int main()
{
    int x = std::rand();
    int y = std::rand();

    Implementation obj1();
    obj1.init(y);
    int r1 = obj1.operate(x);


    Specialisation obj2();
    obj2.init(y);
    int r2 = obj2.operate(x);

    return r1 + r2;
}
