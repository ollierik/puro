#pragma once

template <typename FloatType, class GrainType>
class OffsetWrapper
{
public:
    OffsetWrapper(int offset, GrainType grain) 
        : offset(offset), grain(grain)
    {
    }

    void addNextOutput(Buffer<FloatType>& audio)
    {
        const int n = audio.numSamples;

        // no operations needed for this block
        if (offset >= n)
        {
            offset -= n;
            return;
        }

        // full audio needed for this block
        if (offset == 0)
        {
            grain.addNextOutput(audio);
            return;
        }

        // partial audio needed for this block
        Buffer<FloatType> clipped = audio.clip(offset, n - offset);
        grain.addNextOutput(clipped);
        offset = 0;
    }

    GrainType* get() { return &grain; };

private:
    GrainType grain;
    int offset = 0;
};



template <class FloatType, class AudioSourceType, class EnvelopeType>
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

    void addNextOutput(Buffer<FloatType>& audio)
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

        const int nSource = audioSource.getNextOutput(audio.clip(0, n));

        //envelope.getNextOutput(envelope, i0, i1 - i0);

        // audio source depleted
        if (nSource < n)
        {
            remaining = 0;
        }
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
    
    int remaining;
};
