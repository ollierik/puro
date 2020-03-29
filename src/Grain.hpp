#pragma once

//#include <vector>
//#include "AudioSource.hpp"
//#include "Envelope.hpp"
#include <algorithm>

template <class FloatType, class EnvelopeType>
class GrainTemplate
{
public:

    GrainTemplate(int offset, int lengthInSamples, EnvelopeType&& envelope)
        : envelope(std::move(envelope))
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
            vec[i] += 1.0f * envelope.getNext();
        }
        
        index -= (n-offset);
        offset = 0;
    }

    bool terminated()
    {
        return (index <= 0);
    }

private:

    EnvelopeType envelope;
    
    int offset;
    int index;
};
