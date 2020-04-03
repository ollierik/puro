#pragma once

template <class FloatType, class AudioSourceType, class EnvelopeType>
class GrainTemplate
{
public:

    GrainTemplate(int offset,
                  int lengthInSamples,
                  AudioSourceType audioSource,
                  EnvelopeType envelope)
        : audioSource(audioSource)
        , envelope(envelope)
        , offset(offset)
        , remaining(lengthInSamples)
    {
        //std::cout << "*** Create grain *** offset: " << offset << std::endl;
    }

    void getNextOutput(Buffer<FloatType>& audio, Buffer<FloatType>& envelope, const int numSamples)
    {
        if (depleted())
            return;
        
        // clear before offset
        audio.setAllChannelsTo(0, 0, offset);

        int i0 = offset;
        int i1 = numSamples;

        if (remaining < (i1 - i0))
        {
            i1 = i0 + remaining;
        }

        const int nSource = audioSource.getNextOutput(audio, i0, i1 - i0);
        envelope.getNextOutput(envelope, i0, i1 - i0);

        // audio source depleted
        if (nSource < i1 - i0)
        {
            remaining = 0;
            i1 = i0 + nSource;
        }

        // clear the tail if needed
        audio.setAllChannelsTo(0, i1, numSamples - i1);

        offset = 0;
#if 0
        if (depleted())
            return;

        // TODO:
        // Refactor to clean up
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
#endif
    }

    bool depleted()
    {
        return (remaining <= 0);
    }

    void terminate()
    {
        remaining = 0;
    }

private:

    AudioSourceType audioSource;
    EnvelopeType envelope;
    
    int offset;
    int remaining;
};
