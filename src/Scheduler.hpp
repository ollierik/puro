#pragma once

#include "Engine.hpp"

template <class GrainType>
class SchedulerTemplate
{
public:
    SchedulerTemplate() : period(200), counter(period)
    {}

    void tick(int n)
    {
        std::cout << counter << std::endl;;
        if (counter >= period)
        {
            counter -= period;
            GrainType* g = engine->allocateGrain();
            if (g != nullptr)
            {
                new (g) GrainType(counter, grainLength);
            }
        }
        counter += n;
    }

    void setEngine(EngineInterface<GrainType>* e) { engine = e; }

    EngineInterface<GrainType>* engine;

    const int period;
    int counter;
    const int grainLength = 100;
};
