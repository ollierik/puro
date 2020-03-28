#pragma once

#include "Engine.hpp"

template <class GrainType>
class SchedulerTemplate
{
public:
    SchedulerTemplate() : period(20), counter(0)
    {}

    void tick(int n)
    {
        std::cout << counter << " -> " << counter + n << std::endl;;
        counter += n;
        if (counter >= period)
        {
            counter -= period;
            const int offset = n - counter;
            GrainType* g = engine->allocateGrain();
            if (g != nullptr)
            {
                new (g) GrainType(offset, grainLength);
            }
        }
    }

    void setEngine(EngineInterface<GrainType>* e) { engine = e; }

    EngineInterface<GrainType>* engine;

    const int period;
    int counter;
    const int grainLength = 10;
};
