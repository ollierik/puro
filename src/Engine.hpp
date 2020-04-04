#pragma once



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
            it->addNextOutput(output);

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

    PoolType pool;
};


