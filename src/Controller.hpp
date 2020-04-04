#pragma once

/** Controls parameters and creates grains */
template <class GrainType, class AudioSourceType, class EnvelopeType, class EngineType>
class ControllerTemplate
{
public:

    ControllerTemplate(EngineType& e) : engine(e), counter(interval)
    {
    }

    /** Called from tick, separated for convenience */
    void advance(const int numSamples)
    {
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
            if (createGrain(offset))
            {
                break;
            }
        }
    }

    bool createGrain(int offset)
    {
        GrainType* g = engine.addGrain(offset, duration, AudioSourceType(), EnvelopeType());
        return g == nullptr;
    }

private:

    EngineType& engine;

    int interval = 20;
    int duration = 10;
    int counter;
};
