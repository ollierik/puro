#pragma once

template <typename ValueType>
class Parameter 
{
public:
    Parameter(ValueType v) : value(v) {}

    ValueType getValue() { return value; }
    ValueType setValue(ValueType v) { value = v; }
protected:
    ValueType value;
};


class IntParameter : public Parameter<int>
{
public:
    IntParameter(int initialValue) : Parameter(initialValue) {}
    operator int() const { return value; }
};
