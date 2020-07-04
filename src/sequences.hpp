#pragma once

namespace puro {

template <int increment = 1>
struct IndexSequence
{
    IndexSequence(int val) noexcept : value(val) {}

    static constexpr int increment = increment;

    IndexSequence operator++() noexcept // prefix
    {
        value += increment;
        return value;
    };

    IndexSequence operator++(int) noexcept // postfix
    {
        IndexSequence temp (value, increment);
        value += increment;
        return temp;
    };

    IndexSequence& operator+=(const int rhs) noexcept
    {
        value += rhs;
        return *this;
    }

    operator int() const noexcept { return value; }

    int value;
};



template <typename T>
struct Sequence
{
    Sequence(T val, T inc) : value(val), increment(inc) {}

    Sequence operator++() // prefix
    {
        value += increment;
        return value;
    };

    Sequence operator++(int) // postfix
    {
        Sequence temp (value, increment);
        value += increment;
        return temp;
    };

    operator T() { return value; }

    T value;
    T increment;
};

} // namespace puro
