#include <iostream>
#include <vector>

#include "Engine.hpp"
#include "Pool.hpp"
#include "Grain.hpp"

int main ()
{
    const int n = 2048;
    std::vector<float> output (n, 0.0f);

    using Grain = GrainTemplate<float>;
    using Pool = FixedPool<Grain, 16>;
    using Scheduler = SchedulerTemplate<Grain>;
    using Engine = EngineTemplate<float, Grain, Pool, Scheduler>;

    Engine engine;
    Scheduler scheduler;

    engine.setScheduler(&scheduler);

    const int blockSize = 32;
    for (int i=0; i<n; i+=blockSize)
    {
        engine.tick(&output[i], blockSize);
    }
}
