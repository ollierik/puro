#pragma once



// TODO:
// - How to do this as DRY? Partial specialisations always need the whole class?

/** Primary template, not implemented */
template <typename FloatType, class BlockSizeParameter, class GrainType, class PoolType, class ControllerType>
class EngineTemplate
{
};


/** Partial specialisation, constant buffer size */

template <typename FloatType, int N, class GrainType, class PoolType, class ControllerType>
class EngineTemplate<FloatType, ConstIntParameter<N>, GrainType, PoolType, ControllerType>
{
public:

    EngineTemplate(ConstIntParameter<N>& /*bs*/, ControllerType& c)
        : controller(c)
    {
        controller.bindAllocation(std::bind(&PoolType::allocate, &pool));
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

            if (it->hasTerminated())
                pool.remove(it);
        }
    }

    PoolType& getPool() { return pool; }

private:

    std::array<FloatType, N> audioBuffer;
    std::array<FloatType, N> envelopeBuffer;

    PoolType pool;
    ControllerType& controller;
};



/** Partial specialisation, variable buffer size */

template <typename FloatType, class GrainType, class PoolType, class ControllerType>
class EngineTemplate <FloatType, IntParameter, GrainType, PoolType, ControllerType>
{
public:
    EngineTemplate(IntParameter& blockSizeParameter, ControllerType& c)
        : controller(c), blockSize(blockSizeParameter)
    {
        controller.bindAllocation(std::bind(&PoolType::allocate, &pool));
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
            it->getNextOutput(audioBuffer.data(), envelopeBuffer.data(), numSamples);

            for (int i=0; i<numSamples; ++i)
            {
                output[i] += audioBuffer[i] * envelopeBuffer[i];
            }

            if (it->hasTerminated())
                pool.remove(it);
        }
    }

    PoolType& getPool() { return pool; }

private:

    IntParameter& blockSize;

    std::vector<FloatType> audioBuffer;
    std::vector<FloatType> envelopeBuffer;

    PoolType pool;
    ControllerType& controller;
};


