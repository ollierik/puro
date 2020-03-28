#include <iostream>
#include <vector>

#include "Grain.hpp"
#include "Pool.hpp"
#include "Engine.hpp"
#include "Scheduler.hpp"

int main ()
{
    const int n = 1024;
    std::vector<float> output (n, 0.0f);

    using Grain = GrainTemplate<float>;
    using Pool = FixedPool<Grain, 16>;
    using Scheduler = SchedulerTemplate<Grain>;
    using Engine = EngineTemplate<float, Grain, Pool, Scheduler>;

    Scheduler scheduler;
    Engine engine(scheduler);

    const int blockSize = 32;

    for (int i=0; i<n; i+=blockSize)
    {
        engine.tick(&output[i], blockSize);
    }

    std::cout << "\n    OUTPUT\n----------\n";

    for (int i=0; i<n; i++)
    {
        std::cout << i << ": " << output[i] << std::endl;
    }
}

//#include "godbolt/godbolt_cyclical_template.hpp"
