#pragma once


/** Primary template, not implemented */
template <typename FloatType, class GrainType, class PoolType, class ControllerType>
class EngineTemplate
{
public:
    EngineTemplate(ControllerType& c)
        : controller(c)
    {
        controller.bindAllocation(std::bind(&PoolType::allocate, &pool));
    }

    /** Reserves buffer size for internal buffers.
        Should be called before starting processing with maximum possible input buffer size to avoid allocation during audio loop.
    */
    void reserveBufferSize(int size)
    {
        std::cout << "Resize buffers" << std::endl;
        audioBuffer.resize(size);
        envelopeBuffer.resize(size);
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

            Math::multiplyAdd(output, audioBuffer.data(), envelopeBuffer.data(), numSamples);

            if (it->hasTerminated())
                pool.remove(it);
        }
    }

    PoolType& getPool() { return pool; }

private:

    std::vector<FloatType> audioBuffer;
    std::vector<FloatType> envelopeBuffer;

    PoolType pool;
    ControllerType& controller;
};


