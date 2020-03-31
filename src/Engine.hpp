#pragma once

#include <array>
#include <vector>
#include "Parameter.hpp"

// TODO:
// - How to do this as DRY?

template <class GrainType>
class EngineInterface
{
public:
    virtual GrainType* allocateGrain() = 0;
};

/** Primary template, not implemented */
template <typename FloatType, class BlockSize, class GrainType, class PoolType, class ControllerType>
class EngineTemplate : public EngineInterface<GrainType>
{
};




/** Partial specialisation, constant buffer size */

template <typename FloatType, int N, class GrainType, class PoolType, class ControllerType>
class EngineTemplate<typename FloatType, ConstIntParameter<N>, GrainType, PoolType, ControllerType> : public EngineInterface<GrainType>
{
public:

    EngineTemplate(ConstIntParameter<N>& /*bs*/, ControllerType& c)
        : controller(c)
    {
        controller.setEngine(this);
    }

    /** numSamples provided for convenience and ignored */
    void tick(FloatType* output, int /*numSamples*/)
    {
        // scheduler operation
        controller.tick(N);

        // grain operations
        for (auto& it : pool)
        {
            //it->addNextOutput(output, n);
            it->getNextOutput(audioBuffer.data(), envelopeBuffer.data(), N);

            for (int i=0; i<N; ++i)
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

    std::array<FloatType, N> audioBuffer;
    std::array<FloatType, N> envelopeBuffer;

    PoolType pool;
    ControllerType& controller;
};





/** Partial specialisation, variable buffer size */

template <typename FloatType, class GrainType, class PoolType, class ControllerType>
class EngineTemplate <FloatType, IntParameter, GrainType, PoolType, ControllerType> : public EngineInterface<GrainType>
{
public:
    EngineTemplate(IntParameter& blockSizeParameter, ControllerType& c)
        : controller(c), blockSize(blockSizeParameter)
    {
        controller.setEngine(this);
    }

    /** Reserves buffer size for internal buffers.
        Should be called before starting processing with maximum possible input buffer size to avoid allocation during audio loop.
    */
    void reserveBufferSize(int size)
    {
        std::cout << "Resize buffers" << std::endl;
        audioBuffer.resize(blockSize);
        envelopeBuffer.resize(blockSize);
    }

    /** numSamples provided for convenience and ignored */
    void tick(FloatType* output, int numSamples)
    {
        if (audioBuffer.size() < numSamples)
        {
            reserveBufferSize(numSamples);
        }

        // scheduler operation
        controller.tick(numSamples);

        // grain operations
        for (auto& it : pool)
        {
            //it->addNextOutput(output, n);
            it->getNextOutput(audioBuffer.data(), envelopeBuffer.data(), numSamples);

            for (int i=0; i<numSamples; ++i)
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

    IntParameter& blockSize;

    std::vector<FloatType> audioBuffer;
    std::vector<FloatType> envelopeBuffer;

    PoolType pool;
    ControllerType& controller;
};


