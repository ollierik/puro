#pragma once

#include "puro.hpp"
#include "parameter.hpp"
#include <iomanip>

struct Grain
{
    Grain(int offset, int length, puro::DynamicBuffer<float>& sourceBuffer, int startIndex)
        : ranges(offset, length)
        , audioSrc(sourceBuffer)
        , audioSeq((float)startIndex, 0.5f)
        , envlSeq(puro::envl_halfcos_create_seq<float>(length))
    {}

    engine::Ranges ranges;

    puro::DynamicBuffer<float>& audioSrc;
    //puro::IndexSequence<> audioSeq;
    puro::Sequence<float> audioSeq;

    puro::Sequence<float> envlSeq;
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
	
    auto audio = puro::wrap_vector<BufferType> (context.temp1, output.size());
    //std::tie(audio, grain.audioSeq) = puro::buffer_fill(audio, grain.audioSrc, grain.audioSeq);
    std::tie(audio, grain.audioSeq) = puro::buffer_interp1_fill(audio, grain.audioSrc, grain.audioSeq);

    BufferType envelope = puro::wrap_vector<BufferType> (context.temp2, audio.size());
    //std::tie(envelope, grain.envlSeq) = puro::envl_halfcos_fill(envelope, grain.envlSeq);
    envelope = puro::constant_fill(envelope, 1.0f);

    output = puro::trimmed_length(output, envelope.size());
    output = puro::multiply_add(output, audio, envelope);

    grain.ranges = engine::ranges_advance(grain.ranges, buffer.size());

    return (grain.ranges.remaining <= 0) || (output.size() != numSamplesToWrite);
}

int main()
{
    using BufferType = puro::Buffer<float, 2>;
    std::vector<float> vec;
    BufferType output = puro::wrap_vector<BufferType>(vec, 256);

    Context context;

    SimpleScheduler scheduler (20);

    AlignedPool<Grain> pool;
    pool.elements.reserve(16);

    std::vector<float> audioFileData;
    auto audioFileBuffer = puro::fit_vector_into_dynamic_buffer<puro::DynamicBuffer<float>> (audioFileData, 2, 2000);

    audioFileBuffer = puro::linspace_fill(audioFileBuffer, 0.0f, 1.0f);

    //GaussianParameter<float, int> intervalParameter(40, 0.001f);
    //GaussianParameter<float, int> durationParameter(10, 0.001f);
    //GaussianParameter<float, int> materialOffsetParameter(100, 3);

    const int blockSize = 32;

    for (int i=0; i<output.size(); i+=blockSize)
    {
        BufferType buffer = puro::slice(output, i, blockSize);

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
            //const int interval = intervalParameter.get();
            //const int duration = std::max(durationParameter.get(), 5);
            //const int materialOffset = std::max(materialOffsetParameter.get(), 0);
            const int interval = 100;
            const int duration = 80;
            const int materialOffset = 1980;

            scheduler.interval = interval;
            auto it = pool.push(Grain(blockSize - n, duration, audioFileBuffer, materialOffset));

            if (it.isValid())
            {
                if (process_grain(buffer, it.get(), context))
                    pool.pop(it);
            }
        }
    }

    for (int i=0; i < output.size(); i++)
    {
        std::cout << std::setprecision(4) << std::fixed;
        std::cout << i << ":\t" << output(0, i) << "\t" << output(1, i) << std::endl;;
    }

    return 0;
}
