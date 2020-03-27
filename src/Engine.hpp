#pragma once

class Binding
{
    void tick()


};


template <class GrainType>
class SchedulerTemplate
{
public:

    void tick(int n)
    {
        counter += n;
        if (counter >= period)
        {
            counter -= period;
        }
    }

    int counter = 0;
    int period = 1000;
};

template <typename FloatType, class GrainType, class PoolType, class SchedulerType>
class EngineTemplate
{
public:

    EngineTemplate() = default;

    void tick(FloatType* output, int n)
    {
        // scheduler operation
        scheduler.tick(n);

        // grain operations
        for (auto& it : pool)
        {
            it->addNextOutput(output, n);

            if (it->terminated())
            {
                pool.remove(it);
            }
        }
    }

    template <class ...Args>
    bool addGrain(Args... grainArgs)
    {
        return (pool.add(grainArgs) != nullptr);
    }

private:

    PoolType pool;
    SchedulerType scheduler;
};
