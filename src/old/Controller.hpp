#pragma once

/** Controls parameters and creates grains */
template <typename FloatType, class EngineType, class SoundObjectType, class AudioSourceType, class EnvelopeType>
class GranularController
{
public:

    GranularController(EngineType& e) : engine(e), counter(interval)
    {
    }

    /** Called from tick, separated for convenience */
    void advance(Buffer<FloatType>& output)
    {
        const int numSamples = output.size();
        int samplesRemaining = numSamples;
        while (samplesRemaining > 0)
        {
            counter += samplesRemaining;

            // shouldn't create grain this block
            if (counter <= interval)
            {
                break;
            }

            samplesRemaining = counter - interval;
            counter = 0;

            const int offset = numSamples - samplesRemaining;

            // if we can't create a new grain, stop trying and exit loop
            //if (createGrain(offset, blockSize, output))
            if (createAndRunGrain(output, offset))
            {
                break;
            }
        }
    }

    virtual bool createAndRunGrain(Buffer<FloatType>& output, int offset) = 0;
        /*
    {
        SoundObjectType* s = engine.addSound(offset, duration, AudioSourceType(), EnvelopeType(duration));
        return s  == nullptr;
    }
    */

protected:

    EngineType& engine;

    int interval = 20;
    int duration = 10;
    int counter;
};

/** Controls parameters and creates grains */
template <typename FloatType, class EngineType, class SoundObjectType, class AudioSourceType, class EnvelopeType>
class BufferedGranularController : public GranularController<FloatType, EngineType, SoundObjectType, AudioSourceType, EnvelopeType>
{
public:

    using BaseClass = GranularController<FloatType, EngineType, SoundObjectType, AudioSourceType, EnvelopeType>;

    BufferedGranularController(EngineType& e) : BaseClass(e) {}

    bool createAndRunGrain(Buffer<FloatType>& output, int offset) override
    {
        errorif(audioBuffer == nullptr, "audio buffer not set");
        SoundObjectType* s = this->engine.addAndRunSound(output, offset, this->duration,
                                                        AudioSourceType(*audioBuffer, bufferIndex),
                                                        EnvelopeType(this->duration));
        return s == nullptr;
    }

    void setAudioBuffer(Buffer<FloatType>& buffer)
    {
        audioBuffer = &buffer;
    }

    void setAudioSourceLocation(int indexInSamples)
    {
        bufferIndex = indexInSamples;
    }

private:

    Buffer<FloatType>* audioBuffer;
    int bufferIndex = 0;
};

