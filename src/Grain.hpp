#pragma once


template <typename FloatType>
class GrainContext
{
    std::vector<FloatType> tempAudioBuffer;
    std::vector<FloatType> tempEnvelopeBuffer;
};


template <class FloatType, class ContextType, class AudioSourceType, class EnvelopeType>
class GrainTemplate
{
public:

    GrainTemplate(int lengthInSamples,
                  AudioSourceType audioSource,
                  EnvelopeType envelope)
        : audioSource(audioSource)
        , envelope(envelope)
        , remaining(lengthInSamples)
    {
    }

    void next(Buffer<FloatType>& output, ContextType& context)
    {
        Buffer<FloatType> audioBuffer (output, context.tempAudioBuffer);
        SourceOperations::replace(audioBuffer, audioSource);

        Buffer<FloatType> envelopeBuffer (audioBuffer, context.tempEnvelopeBuffer);
        SourceOperations::replace(envelopeBuffer, envelopeSource);

        SourceOperations::multiplyAdd(output, audioBuffer, envelopeBuffer);
    }

#if 0
    void addNextOutput(Buffer<FloatType>& audio, Context<FloatType> context)
    {
        if (depleted())
            return;

        int n = audio.numSamples;

        if (remaining < n)
        {
            n = remaining;
            remaining = 0;
        }
        else
        {
            remaining -= n;
        }

        const int nSource = audioSource.addNextOutput<Context<FloatType>>(audio.clip(0, n));

        //envelope.getNextOutput(envelope, i0, i1 - i0);

        // audio source depleted
        if (nSource < n)
        {
            remaining = 0;
        }
    }
#endif

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
    
    int remaining;
};
