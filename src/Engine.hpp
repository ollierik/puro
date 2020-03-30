#pragma once

#include <vector>
#include <type_traits>
#include "Buffer.hpp"
#include "Parameter.hpp"

template <class GrainType>
class EngineInterface
{
public:
    virtual GrainType* allocateGrain() = 0;
};

template <typename FloatType, class SizeParamType, const SizeParamType& SizeParam, class GrainType, class PoolType, class ControllerType>
class EngineTemplate : public EngineInterface<GrainType>
{
public:

    EngineTemplate(ControllerType& c)
        : controller(c)
    {
        controller.setEngine(this);
    }

    void tick(FloatType* output, int numSamples)
    {
        // scheduler operation
        controller.tick(numSamples);

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

private:

    const SizeParamType blockSize;

    BufferTemplate<FloatType, SizeParamType> audioBuffer;
    BufferTemplate<FloatType, SizeParamType> envelopeBuffer;
    //BufferTemplate<FloatType, decltype(blockSize)> audioBuffer;
    //BufferTemplate<FloatType, decltype(blockSize)> envelopeBuffer;

    PoolType pool;
    ControllerType& controller;
};
