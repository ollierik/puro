#pragma once

template <class GrainType>
class EngineInterface
{
public:
    virtual GrainType* allocateGrain() = 0;
};

template <typename FloatType, class GrainType, class PoolType, class SchedulerType>
class EngineTemplate : public EngineInterface<GrainType>
{
public:

    EngineTemplate(SchedulerType& s)
        : scheduler(s)
    {
        scheduler.setEngine(this);
    }

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

    GrainType* allocateGrain() override
    {
        return pool.allocate();
    }

   bool testFunction(int i)
   {
       return i == 0;
   }

private:

    PoolType pool;
    SchedulerType& scheduler;
};
