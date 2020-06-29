//#include "02_granular_example.hpp"

#include "../../benchmark/include/benchmark/benchmark.h"

#ifdef _WIN32
#pragma comment ( lib, "Shlwapi.lib" )
#ifdef _DEBUG
#pragma comment ( lib, "benchmarkd.lib" )
#else
#pragma comment ( lib, "benchmark.lib" )
#endif
#endif

#include "puro.hpp"

struct Grain
{
    Grain(int offset, int length, int startIndex)
        : ranges(offset, length)
        , envelopeRange(puro::envelope_halfcos_create_range<float>(length))
    {}

    engine::Ranges ranges;
    puro::RatioRange<float> envelopeRange;
};

#define POOL_SIZE 128

static void BM_AlignedPool(benchmark::State& state)
{
    AlignedPool<Grain> pool;
    pool.elements.reserve(POOL_SIZE);

    for (auto _ : state)
    {
        for (int i = 0; i < POOL_SIZE; ++i)
        {
            pool.push(Grain(i, i, i));
        }

        for (auto&& it : pool)
        {
            pool.pop(it);
        }
    }
}

static void BM_NodeStack(benchmark::State& state)
{
    SafeStack<Grain> inactive;
    NodeStack<Grain> active;
    SafeStack<Grain> added;
    NodeStack<Grain> removed;

    for (int i=0; i<POOL_SIZE; i++)
    {
        inactive.push_front(new Node<Grain>());
    }

    for (auto _ : state)
    {
        for (int i = 0; i < POOL_SIZE/4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                auto* node = inactive.pop_front();
                if (node != nullptr)
                {
                    auto& e = node->getElement();
                    e = Grain(i, i, i);
                    added.push_front(node);
                }
            }

            active.push_multiple(added.pop_all());
        }

        int counter = 0;
        for (auto&& it : active)
        {
            removed.push_front(active.pop(it));
            if (++counter == 4)
            {
                inactive.push_multiple(removed.pop_all());
                counter = 0;
            }
        }
    }

}


//BENCHMARK(BM_PuroFull);
BENCHMARK(BM_AlignedPool);
BENCHMARK(BM_NodeStack);

BENCHMARK_MAIN();