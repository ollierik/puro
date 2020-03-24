#pragma once

#include <cstdlib>
#include <math.h>

class Operation
{
public:
    virtual int perform(int x, int y) = 0;
};

class DivOperation : public Operation
{
public:
    int perform(int x, int y) { return x / y; }
};

class PowOperation : public Operation
{
public:
    int perform(int x, int y) { return pow(x, y); }
};

template <class OpType>
class TemplatedStackObject
{
public:

    TemplatedStackObject(int x) : x(x) {}

    int doStuff(int y)
    {
        return operation.perform(x, y);
    }

    int x;
    OpType operation; 
};

int main()
{
    int x = std::rand();
    int y = std::rand();

    TemplatedStackObject<DivOperation> obj1 (x);
    int r1 = obj1.doStuff(y);

    TemplatedStackObject<PowOperation> obj2 (x);
    int r2 = obj2.doStuff(y);

    return r1 + r2;
}
