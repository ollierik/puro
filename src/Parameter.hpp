#pragma once

template <typename ValueType, ValueType Value>
class ConstParameter
{
public:
    constexpr ValueType getValue() const { return value; };
private:
    const ValueType value = Value;
};

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


template <int Value>
class ConstIntParameter : public ConstParameter<int, Value>
{
public:
    operator int() const { return Value; }
};


class IntParameter : public Parameter<int>
{
public:
    IntParameter(int initialValue) : Parameter(initialValue) {}
    operator int() const { return value; }
};
