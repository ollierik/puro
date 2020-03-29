#pragma once

#include "structs.hpp"
#include <cstdlib>

template <typename FloatType>
class AudioSourceTemplate
{
public:

    AudioSourceTemplate() = default;

    void getNextOutput(FloatType* vec, int numSamples)
    {
        for (int i=0; i<numSamples; i++)
        {
            const FloatType sample = static_cast<FloatType> (std::rand()) / static_cast<FloatType> (RAND_MAX);
            vec[i] = sample;
        }
    }
};
