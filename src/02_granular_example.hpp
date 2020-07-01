#pragma once

#include "puro.hpp"
#include "parameter.hpp"
#include <iomanip>

struct Grain
{
    Grain(int offset, int length, puro::DynamicBuffer<float>& sourceBuffer, int startIndex)
        : range(offset, length)
        , audioSrc(sourceBuffer)
        , audioSeq((float)startIndex, 1.0f)
        , envlSeq(puro::envl_halfcos_create_seq<float>(length))
    {}

    puro::Range range;
    puro::DynamicBuffer<float>& audioSrc;
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
	BufferType output = puro::range_crop_buffer(grain.range, buffer);
    const int numSamplesToWrite = output.size();
	
    auto audio = puro::wrap_vector<BufferType> (context.temp1, output.size());
    audio = puro::interp_crop_buffer(audio, grain.audioSrc.size(), grain.audioSeq, 1);
    std::tie(audio, grain.audioSeq) = puro::buffer_interp1_fill(audio, grain.audioSrc, grain.audioSeq);

    BufferType envelope = puro::wrap_vector<BufferType> (context.temp2, audio.size());
    std::tie(envelope, grain.envlSeq) = puro::envl_halfcos_fill(envelope, grain.envlSeq);
    //envelope = puro::constant_fill(envelope, 1.0f);

    output = puro::trimmed_length(output, envelope.size());
    output = puro::multiply_add(output, audio, envelope);

    grain.range = puro::range_advance(grain.range, buffer.size());

    return (grain.range.remaining <= 0) || (output.size() != numSamplesToWrite);
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
