#include <vector>
#include <iostream>

#include "defs.hpp"
#include "math.hpp"
#include "pool.hpp"
#include "buffer.hpp"
#include "sources.hpp"
#include "engine.hpp"

struct Scheduler
{
    int tick(int numSamples)
    {
        counter += numSamples;

        if (counter <= interval)
        {
            return 0;
        }

        numSamples = counter - interval;
        counter = 0;

        return numSamples;
    }

    int interval = 20;
    int counter = 0;
};

struct Grain
{
    Grain(int offset, int length)
        : ranges(offset, length)
        , envelope(length)
        , source()
    {}

    engine::Ranges ranges;
    HannEnvelope<float> envelope;
    ConstSource<float> source;
};

struct Context
{
    std::vector<float> temp1;
    std::vector<float> temp2;
};

template <typename BufferType, typename ElementType, typename ContextType>
bool process_grain(const BufferType& buffer, ElementType& grain, ContextType& context)
{
	BufferType output = engine::ranges_crop_buffer(grain.ranges, buffer);
	
	BufferType temp1 (output, context.temp1);
    ops::buffer_fill_from_source(temp1, grain.source);

	BufferType temp2 (temp1, context.temp2);

    ops::buffer_fill_from_source(temp2, grain.envelope);

    output.trimLengthToMatch(temp2);

    ops::buffer_multiply_add(output, temp1, temp2);

    engine::ranges_advance(grain.ranges, buffer.size());

    return (grain.ranges.remaining <= 0);
}

int main()
{
    std::vector<float> vec;
    Buffer<float> output (1, 2048, vec);

    Context context;

    Scheduler scheduler;

    const int blockSize = 32;

    Pool<Grain> pool;
    pool.elements.reserve(4);

    for (int i=0; i<output.size(); i+=blockSize)
    {
        Buffer<float> buffer (1, blockSize, &vec[i]);

        for (auto&& it : pool)
        {
            if (process_grain(buffer, it.get(), context))
            {
                pool.pop(it);
            }
        }

        Grain* added = nullptr;
        int n = blockSize;
        while (n = scheduler.tick(n))
        {
            std::cout << "add" << std::endl;
            auto* g = pool.push(Grain(blockSize - n, 10));

            if (g != nullptr)
                process_grain(buffer, *g, context);
        }
    }

    for (int i=0; i<vec.size(); i++)
    {
        std::cout << i << ": " << vec[i] << std::endl;
    
    }


    return 0;
}

