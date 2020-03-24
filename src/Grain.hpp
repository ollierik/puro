#pragma once

#include <vector>

#include "AudioObject.hpp"
#include "AudioSource.hpp"
#include "Envelope.hpp"
#include "structs.hpp"

template <class FloatType>
class Grain : public AudioObject<FloatType>
{
public:

    using Buffer = std::vector<FloatType>;

    Grain(PlaybackInfo& info,
                  double lengthInSeconds,
                  AudioSource<FloatType> audioSource)
        : AudioObject(info)
    {
    }

    void addNextOutput(FloatType* vec) override
    {
        for (int i=0; i<info.bs; i++)
        {
            vec[i] += 1;
        }
    };

private:
    
    const PlaybackInfo& info;
    const int lengthInSamples;
};
