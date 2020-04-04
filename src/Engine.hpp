#pragma once

template <typename FloatType, class GrainType>
class OffsetWrapper
{
public:
    OffsetWrapper(int offset, GrainType grain) 
        : offset(offset), grain(grain)
    {
    }

    void addNextOutput(Buffer<FloatType>& audio, Context<FloatType>& context)
    {
        const int n = audio.numSamples;

        // no operations needed for this block
        if (offset >= n)
        {
            offset -= n;
            return;
        }

        // full audio needed for this block
        if (offset == 0)
        {
            grain.addNextOutput(audio, context);
            return;
        }

        // partial audio needed for this block
        Buffer<FloatType> clipped = audio.clip(offset, n - offset);
        grain.addNextOutput(clipped, context);
        offset = 0;
    }

    GrainType* get() { return &grain; };

private:
    GrainType grain;
    int offset = 0;
    int remaining;
};


template <typename FloatType, class GrainType, class PoolType, class WrapperType>
class EngineTemplate
{
public:
    EngineTemplate() = default;

    void addNextOutput(Buffer<FloatType>& output)
    {

        // grain operations
        for (auto& it : pool)
        {
            it->addNextOutput(output, context);

            if (it->get()->depleted())
                pool.remove(it);
        }
    }

    template <typename... Args>
    GrainType* addGrain(int offsetFromBlockStart, Args... grainArgs)
    {
        WrapperType* w = pool.allocate();
        if (w != nullptr)
        {
            new (w) WrapperType (offsetFromBlockStart, GrainType(grainArgs...));

            return w->get();
        }

        return nullptr;
    }

    GrainTemplate<FloatType, ConstSource, HannEnvelope>::Context<FloatType> context;
    FixedPool<WrapperType, 4> pool;
};


