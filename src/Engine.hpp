#pragma once

/** Primary template, not implemented */
template <typename FloatType, class GrainType, class PoolType, class ControllerType>
class EngineTemplate
{
public:
    EngineTemplate(ControllerType& c) : controller(c)
    {
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

    void tick(FloatType* output, int blockSize)
    {
        if (audioBuffer.size() < blockSize)
        {
            reserveBufferSize(blockSize);
        }

        // grain operations
        for (auto& it : pool)
        {
            it->getNextOutput(audioBuffer.data(), envelopeBuffer.data(), blockSize);

            Math::multiplyAdd(output, audioBuffer.data(), envelopeBuffer.data(), blockSize);

            if (it->depleted())
                pool.remove(it);
        }

        schedule(blockSize);
    }

    PoolType& getPool() { return pool; }

private:

    /** Called from tick, separated for convenience */
    void schedule(int blockSize)
    {
        int samplesRemaining = blockSize;
        while (samplesRemaining > 0)
        {
            counter += samplesRemaining;

            // shouldn't create grain this block
            if (counter <= controller.getInterval())
            {
                break;
            }

            samplesRemaining = counter - controller.getInterval();
            counter = 0;

            const int offset = blockSize - samplesRemaining;

            // if we can't create a new grain, stop trying and exit loop
            if (! createGrain(offset, blockSize))
            {
                break;
            }
        }
    }

    /** Returns false if no new grains can be allocated */
    bool createGrain(int offset, int blockSize)
    {
        GrainType* g = pool.allocate();
        if (g != nullptr)
        {
            //controller.createGrain(g, offset);

            // run grain manually for the first block
            // grain depletion is checked with the next block normally
            //g->getNextOutput(audioBuffer.data(), envelopeBuffer.data(), blockSize);

            return true;
        }
        return false;
    }

    PoolType pool;
    ControllerType& controller;

    std::vector<FloatType> audioBuffer;
    std::vector<FloatType> envelopeBuffer;

    int counter;
};


