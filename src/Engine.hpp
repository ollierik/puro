#pragma once

template <typename FloatType, class ProcessorType, class ProcessorContextType>
class SoundObject
{
public:
    SoundObject(int offset, int lengthInSamples, ProcessorType processor) 
        : processor(processor)
        , offset(offset)
        , remaining(lengthInSamples)
    {
    }

    void addNextOutput(Buffer<FloatType>& buffer, ProcessorContextType& context)
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
        processor.next(buffer, context);

        // if grain changed the numSamples, one of the sources was depleted
        if (buffer.numSamples != numSamplesRequested)
        {
            remaining = 0;
            return;
        }

        remaining -= numSamplesRequested;
        offset = 0;
    }

    ProcessorType* getProcessor() { return &processor; };
    bool depleted() { return remaining <= 0; }
    void terminate() { remaining = 0; }

private:
    ProcessorType processor;
    int offset = 0;
    int remaining;
};

template <typename FloatType, class SoundObjectType, class ProcessorType, class ProcessorContextType, class PoolType>
class SoundObjectEngine
{
public:
    SoundObjectEngine() = default;

    void addNextOutput(Buffer<FloatType>& output)
    {
        for (auto& it : pool)
        {
            Buffer<FloatType> range = output;
            it->addNextOutput(range, context);

            if (it->depleted())
                pool.remove(it);
        }
    }

    template <typename... ProcessorArgs>
    SoundObjectType* addSound(int offsetFromBlockStart, int lengthInSamples, ProcessorArgs... processorArgs)
    {
        SoundObjectType* s = pool.allocate();
        if (s != nullptr)
        {
            new (s) SoundObjectType (offsetFromBlockStart, lengthInSamples, ProcessorType(processorArgs...));

            return s;
        }

        return nullptr;
    }

    ProcessorContextType context;
    PoolType pool;
};


