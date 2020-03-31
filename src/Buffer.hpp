#pragma once

#include <array>
#include <vector>
#include <type_traits>

#include "Parameter.hpp"

#include "definitions.hpp"

/*
template <typename FloatType, class Param>
class BufferTemplate
{
};

/** std::array*/

#if 0

template <typename FloatType, int N>
class BufferTemplate <FloatType, ConstIntParameter<N>>
{
public:

    FloatType* getRaw() { return &buffer[0]; }

    constexpr size_t size() { return N; }

    FloatType& operator[] (int i)
    {
        passert(i < 0 || i >= buffer.size(), "Index out of range");
        return buffer[i];
    }

private:
    std::array<FloatType, N> buffer;
};


/** std::vector */
template <typename FloatType>
class BufferTemplate <FloatType, IntParameter>
{
    BufferTemplate()
    {
        buffer.resize(32, 0.0);
    }

    FloatType* getRaw() { return &buffer[0]; }

    size_t size() { return buffer.size(); }

    FloatType& operator[] (int i)
    {
        passert(i < 0 || i >= buffer.size(), "Index out of range");
        return buffer[i];
    }

private:
    std::vector<FloatType> buffer;
};

#endif
