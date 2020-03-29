#pragma once

#include "Engine.hpp"
#include <utility>

template <class GrainType, class AudioSourceType, class EnvelopeType>
class SchedulerTemplate
{
public:
    SchedulerTemplate() : period(20), counter(0)
    {}

    void tick(int n)
    {
        // TODO:
        // refactor into not using a per-sample loop
        int i = 0;
        while (i<n)
        {
            if (counter <= 0)
            {
                createGrain(i);
                counter = period;
            }
            ++i;
            --counter;
        }
    }

    void createGrain(int offset)
    {
        GrainType* g = engine->allocateGrain();
        if (g != nullptr)
        {
            new (g) GrainType(offset,
                grainLength,
                std::move(AudioSourceType()),
                std::move(EnvelopeType(grainLength)));
        }
        else
        {
            std::cout << "Couldn't create a new grain\n";
        }
    }

    void setEngine(EngineInterface<GrainType>* e) { engine = e; }

    EngineInterface<GrainType>* engine;

    const int period;
    int counter;
    const int grainLength = 10;
};
