#pragma once

template <typename ValueType, ValueType Value>
class ConstParameter 
{
public:

    constexpr ValueType getValue()
    {
        return Value;
    }
};

template <int Value>
class ConstIntParameter : public ConstParameter<int, Value>
{
};

template <typename ValueType>
class MutableParameter 
{
public:
    ValueType getValue() override
    {
        return value;
    }

    void setValue(ValueType newValue)
    {
        value = newValue;
    }

private:
    ValueType& value;
};
