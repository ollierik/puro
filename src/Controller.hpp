#pragma once

template <class GrainType, class AudioSourceType, class EnvelopeType>
class ControllerTemplate
{
public:
    ControllerTemplate() : period(20), counter(0)
    {}

    void tick(int n)
    {
        int samplesRemaining = n;
        while (samplesRemaining > 0)
        {
            counter += samplesRemaining;
            if (counter > period)
            {
                samplesRemaining = counter - period;
                counter = 0;

                const int offset = n - samplesRemaining;
                // if we can't create a new grain, stop trying to add grains and exit loop
                if (!createGrain(offset))
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }

    bool createGrain(int offset)
    {
        GrainType* g = allocationCallback();
        if (g != nullptr)
        {

            new (g) GrainType(offset,
                grainLength,
                audioSourceFactoryCallback(),
                EnvelopeType(grainLength));

            return true;
        }

        //std::cout << "Couldn't create a new grain\n";
        return false;
    }

    void bindAllocation(std::function<GrainType* ()> callback)
    {
        allocationCallback = callback;
    }

    void bindAudioSourceFactory(std::function<AudioSourceType()> callback)
    {
        audioSourceFactoryCallback = callback;
    }

    std::function<GrainType* ()> allocationCallback;
    std::function<AudioSourceType ()> audioSourceFactoryCallback;

    const int period;
    int counter;
    const int grainLength = 10;
};
