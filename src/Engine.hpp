#pragma once

#include <vector>
#include "Buffer.hpp"

template <class GrainType>
class EngineInterface
{
public:
    virtual GrainType* allocateGrain() = 0;
};

template <typename FloatType, class BufferType, class GrainType, class PoolType, class SchedulerType>
class EngineTemplate : public EngineInterface<GrainType>
{
public:

    EngineTemplate(SchedulerType& s)
        : scheduler(s)
    {
        scheduler.setEngine(this);
    }

    void tick(FloatType* output, int numSamples)
    {
        // scheduler operation
        scheduler.tick(numSamples);

        // grain operations
        for (auto& it : pool)
        {
            //it->addNextOutput(output, n);
            it->getNextOutput(audioBuffer.getRaw(), envelopeBuffer.getRaw(), numSamples);

            for (int i=0; i<numSamples; i++)
            {
                output[i] += audioBuffer[i] * envelopeBuffer[i];
            }

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

    BufferType audioBuffer;
    BufferType envelopeBuffer;

    PoolType pool;
    SchedulerType& scheduler;
};
