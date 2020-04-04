#pragma once

template <typename FloatType, class GrainType, class ContextType>
class OffsetWrapper
{
public:
    OffsetWrapper(int offset, int lengthInSamples, GrainType grain) 
        : grain(grain)
        , offset(offset)
        , remaining(lengthInSamples)
    {
    }

    void addNextOutput(Buffer<FloatType>& buffer, ContextType& context)
    {
        if (depleted())
            return;

        // no operations needed for this block
        if (offset >= buffer.numSamples)
        {
            offset -= buffer.numSamples;
            return;
        }

        if (offset > 0)
            buffer.trimBegin(offset);

        // restrict range if grain should terminate this block
        if (remaining < buffer.numSamples)
            buffer.trimLength(remaining);

        const int numSamplesRequested = buffer.numSamples;
        grain.next(buffer, context);

        // if grain changed the numSamples, one of the sources was depleted
        if (buffer.numSamples != numSamplesRequested)
        {
            remaining = 0;
            return;
        }

        remaining -= numSamplesRequested;
        offset = 0;
    }

    GrainType* get() { return &grain; };
    bool depleted() { return remaining <= 0; }
    void terminate() { remaining = 0; }

private:
    GrainType grain;
    int offset = 0;
    int remaining;
};


template <typename FloatType, class GrainType, class PoolType, class WrapperType, class ContextType>
class EngineTemplate
{
public:
    EngineTemplate() = default;

    void addNextOutput(Buffer<FloatType>& output)
    {

        // grain operations
        for (auto& it : pool)
        {
            Buffer<FloatType> range = output;
            it->addNextOutput(range, context);

            if (it->depleted())
                pool.remove(it);
        }
    }

    template <typename... Args>
    GrainType* addGrain(int offsetFromBlockStart, int lengthInSamples, Args... grainArgs)
    {
        WrapperType* w = pool.allocate();
        if (w != nullptr)
        {
            new (w) WrapperType (offsetFromBlockStart, lengthInSamples, GrainType(grainArgs...));

            return w->get();
        }

        return nullptr;
    }

    ContextType context;
    PoolType pool;
};


