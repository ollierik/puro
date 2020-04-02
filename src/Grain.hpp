#pragma once

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
        //std::cout << "*** Create grain *** offset: " << offset << std::endl;
    }

    void getNextOutput(FloatType* audioBuffer, FloatType* envelopeBuffer, int numSamples)
    {
        // TODO:
        // Refactor to clean up
        // Refactor to use SIMD wrapper?
        const int indexFirst = offset;
        const int indexLast = (offset + index > numSamples) ? numSamples : offset + index;
        const int n = indexLast - indexFirst;

        // clear the beginning of the block if needed
        for (int i=0; i<offset; ++i)
        {
            audioBuffer[i] = 0;
            envelopeBuffer[i] = 0;
        }

        // get the relevant content from envelope and audiosource
        const int numSamplesFromSource = audioSource.getNextOutput(&audioBuffer[offset], n);
        envelope.getNextOutput(&envelopeBuffer[offset], n);

        // audio buffer ended or changed
        if (numSamplesFromSource != n)
        {
            // clear the tail
            for (int i=indexLast; i < indexFirst + numSamplesFromSource; ++i)
            {
                audioBuffer[i] = 0;
                envelopeBuffer[i] = 0;
            }

            index = 0;
            return;
        }

        // clear the tail if needed
        for (int i=indexLast; i < numSamples; ++i)
        {
            audioBuffer[i] = 0;
            envelopeBuffer[i] = 0;
        }
        
        index -= (numSamples-offset);
        offset = 0;
    }

    bool hasTerminated()
    {
        return (index <= 0);
    }

    void terminate()
    {
        index = 0;
    }

private:

    AudioSourceType audioSource;
    EnvelopeType envelope;
    
    int offset;
    int index;
};
