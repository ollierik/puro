#pragma once

/** Controls parameters and creates grains */
template <class GrainType, class AudioSourceType, class EnvelopeType>
class ControllerTemplate
{
public:

    void createGrain(GrainType* mem, int offset)
    {
        new (mem) GrainType(offset,
            duration,
            AudioSourceType(),
            EnvelopeType(duration));
    }

    int getInterval()
    {
        return interval;
    }

private:

    int interval = 20;
    int duration = 10;
};
