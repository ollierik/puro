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

    void getNextOutput(FloatType* audioBuffer, FloatType* envelopeBuffer, int numSamples)
    {
        // TODO:
        // SIMD compatibility?
        // Refactor to work with buffers instead of single samples
        const int indexFirst = offset;
        const int indexLast = (offset + index > numSamples) ? numSamples : offset + index;

        // clear the beginning of the block if needed
        for (int i=0; i<offset; ++i)
        {
            audioBuffer[i] = 0;
            envelopeBuffer[i] = 0;
        }

        // get the relevant content from envelope and audiosource
        const int n = indexLast - indexFirst;
        audioSource.getNextOutput(&audioBuffer[offset], n);
        envelope.getNextOutput(&envelopeBuffer[offset], n);

        // clear the tail if needed
        for (int i=indexLast; i < numSamples; ++i)
        {
            audioBuffer[i] = 0;
            envelopeBuffer[i] = 0;
        }
        
        index -= (numSamples-offset);
        offset = 0;
    }

    void getNextEnvelopeOutput(FloatType* vec, int n)
    {
        const int i0 = offset;
        const int i1 = (offset + index > n) ? n : offset + index;

        for (int i=i0 ; i<i1; ++i)
        {
            vec[i] += audioSource.getNext() * envelope.getNext();
        }
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
