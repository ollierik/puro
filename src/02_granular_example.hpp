#pragma once
#include "puro.hpp"

struct Grain
{
    Grain(int offset, int length)
        : ranges(offset, length)
        , source()
        , envelope(length)
    {}

    engine::Ranges ranges;
    NoiseSource<float> source;
    SineEnvelope<float> envelope;
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
    const int numSamplesToWrite = output.size();
	
    BufferType audioBuffer = bops::fit_vector<BufferType>(context.temp1, output.size());
    audioBuffer = bops::filled_from_source(audioBuffer, grain.source);

    BufferType envelopeBuffer = bops::fit_vector<BufferType>(context.temp2, audioBuffer.size());
    envelopeBuffer = bops::filled_from_source(envelopeBuffer, grain.envelope);

    output = bops::trimmed_length(output, envelopeBuffer.size());
    output = bops::multiply_add(output, audioBuffer, envelopeBuffer);

    grain.ranges = engine::ranges_advance(grain.ranges, buffer.size());

    return (grain.ranges.remaining <= 0) || (output.size() != numSamplesToWrite);
}

int main()
{
    using BufferType = Buffer<float, 1>;
    std::vector<float> vec;
    BufferType output = bops::fit_vector<BufferType>(vec, 256);

    Context context;

    SimpleScheduler scheduler (20);

    AlignedPool<Grain> pool;
    pool.elements.reserve(16);

    const int blockSize = 32;

    for (int i=0; i<output.size(); i+=blockSize)
    {
        Buffer<float, 1> buffer (blockSize, &vec[i]);

        for (auto&& it : pool)
        {
            if (process_grain(buffer, it.get(), context))
            {
                pool.pop(it);
            }
        }

        int n = blockSize;
        while (n = scheduler.tick(n))
        {
            scheduler.interval = Parameter
            auto it = pool.push(Grain(blockSize - n, 10));

            if (it.isValid())
            {
                if (process_grain(buffer, it.get(), context))
                    pool.pop(it);
            }
        }
    }

    for (int i=0; i<vec.size(); i++)
    {
        std::cout << i << ": " << vec[i] << std::endl;
    }

    return 0;
}
