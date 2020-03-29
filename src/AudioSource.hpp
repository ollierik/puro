#pragma once

#include "structs.hpp"
#include <cstdlib>

template <typename FloatType>
class AudioSourceTemplate
{
public:

    AudioSourceTemplate() = default;

    FloatType getNext()
    {
        const FloatType sample = static_cast<FloatType> (std::rand()) / static_cast<FloatType> (RAND_MAX);
        return sample;
    }
};
