#pragma once

#include <algorithm>

template <class FloatType, class AudioSourceType, class EnvelopeType>
class GrainTemplate
{
public:

    GrainTemplate(int offset,
                  int lengthInSamples,
                  AudioSourceType&& audioSource,
                  EnvelopeType&& envelope)
        : audioSource(std::move(audioSource))
        , envelope(std::move(envelope))
        , offset(offset)
        , index(lengthInSamples)
    {
        std::cout << "*** Create grain *** offset: " << offset << std::endl;
    }

    void addNextOutput(FloatType* vec, int n)
    {
        // TODO SIMD compatibility?
        const int i0 = offset;
        const int i1 = (offset + index > n) ? n : offset + index;

        for (int i=i0 ; i<i1; ++i)
        {
            vec[i] += audioSource.getNext() * envelope.getNext();
        }
        
        index -= (n-offset);
        offset = 0;
    }

    bool terminated()
    {
        return (index <= 0);
    }

private:

    AudioSourceType audioSource;
    EnvelopeType envelope;
    
    int offset;
    int index;
};
