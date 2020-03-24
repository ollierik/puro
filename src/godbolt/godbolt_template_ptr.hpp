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
class TemplatedObject
{
public:

    TemplatedObject(int x, OpType* op) : x(x), operation(op) {}

    int doStuff(int y)
    {
        return operation->perform(x, y);
    }

    int x;
    OpType* operation; 
};

int main()
{
    int x = std::rand();
    int y = std::rand();

    DivOperation* divOp = new DivOperation();
    PowOperation* powOp = new PowOperation();

    TemplatedObject<DivOperation> obj1 (x, divOp);
    int r1 = obj1.doStuff(y);

    TemplatedObject<PowOperation> obj2 (x, powOp);
    int r2 = obj2.doStuff(y);

    delete divOp;
    delete powOp;

    return r1 + r2;
}
