#pragma once

#include <array>
#include <vector>
#include <type_traits>

#include "definitions.hpp"

template <typename FloatType, class SizeParam>
class BufferTemplate
{
    typedef typename std::conditional<
        std::is_same<SizeParam, ConstIntParameter>,
        std::array<FloatType, 32>,
        std::vector>
        ::type ArrayType;

    ArrayType buffer;
};


#if 0
/** Wraps a buffer object, either  std::array or  std::vector */
template <typename FloatType, class MemoryType>
class BufferTemplate
{
public:

    FloatType* getRaw()
    {
        return &buffer[0];
    }

    size_t size()
    {
        return buffer.size();
    }

    FloatType& operator[] (int i)
    {
        passert(i < 0 || i >= buffer.size(), "Index out of range");
        return buffer[i];
    }

private:

    MemoryType buffer;
};

/*
template <typename FloatType, int BufferSize>
class ConstantBuffer : public BufferTemplate<FloatType, std::array<FloatType, BufferSize>>
{
};
*/

/*
template <class Parameter>
class ConstantBuffer : public BufferTemplate<float, std::array<float, Parameter>>
{
};

template <typename FloatType>
class ResizeableBuffer : public BufferTemplate<FloatType, std::vector<FloatType>>
{
    // TODO allocating and resize
};
*/
#endif
