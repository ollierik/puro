#pragma once

#include "definitions.hpp"

template <class FloatType>
class Envelope
{
public:

    Envelope(const PlaybackInfo& info, int lengthInSamples)
        : lengthInSamples(lengthInSamples)
        , increment(1.0 / static_cast<double>(lengthInSamples))
        , position(0)
    {
    }

    void getNextSamples(FloatType* vec)
    {
        for (int i=0; i<info.bs; i++)
        {
            const FloatType y = std::sin<FloatType> (static_cast<FloatType> (position * M_PI));
            vec[i] = y;
            position += increment;
        }
    }

private:

    const PlaybackInfo& info;
    const int lengthInSamples;

    const int increment;
    double position;
};
