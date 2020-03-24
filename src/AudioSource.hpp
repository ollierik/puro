#pragma once

#include "structs.hpp"
#include <cstdlib>

template <typename FloatType>
class AudioSource
{
public:

    AudioSource(const PlaybackInfo& info)
        : info(info)
    {}

    int getNextSamples(FloatType* vec)
    {
        for (int i=0; i<info.blockSize; i++)
        {
            FloatType sample = static_cast<FloatType> (std::rand() / RAND_MAX);
            vec[i] = sample;
        }
    }

    const PlaybackInfo& info;
    const double location;
};
