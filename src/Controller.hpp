#pragma once

/** Controls parameters and creates grains */
template <typename Float, class Engine, class SoundObject,
    class AudioSource, class Envelope, class Interpolator>
class GranularController
{
public:

    GranularController(Engine& e) : engine(e), counter(interval)
    {
    }

    /** Called from tick, separated for convenience */
    void advance(Buffer<Float>& output)
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

    virtual bool createAndRunGrain(Buffer<Float>& output, int offset) = 0;
        /*
    {
        SoundObject* s = engine.addSound(offset, duration, AudioSource(), Envelope(duration));
        return s  == nullptr;
    }
    */

protected:

    Engine& engine;

    int interval = 20;
    int duration = 10;
    int counter;
};

/** Controls parameters and creates grains */
template <typename Float, class Engine, class SoundObject,
    class AudioSource, class Envelope, class Interpolator>
class BufferedGranularController : public GranularController<Float, Engine, SoundObject, AudioSource, Envelope, Interpolator>
{
public:

    using BaseClass = GranularController<Float, Engine, SoundObject, AudioSource, Envelope, Interpolator>;

    BufferedGranularController(Engine& e) : BaseClass(e) {}

    bool createAndRunGrain(Buffer<Float>& output, int offset) override
    {
        errorif(audioBuffer == nullptr, "audio buffer not set");
        SoundObject* s = this->engine.addAndRunSound(output, offset, this->duration,
                                                        AudioSource(*audioBuffer, bufferIndex),
                                                        //Envelope(this->duration),
                                                        //AudioSource(), Envelope(),
                                                        Envelope(),
                                                        Interpolator(1.0));
        return s == nullptr;
    }

    void setAudioBuffer(Buffer<Float>& buffer)
    {
        audioBuffer = &buffer;
    }

    void setAudioSourceLocation(int indexInSamples)
    {
        bufferIndex = indexInSamples;
    }

private:

    Buffer<Float>* audioBuffer;
    int bufferIndex = 0;
};

