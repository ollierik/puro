#pragma once

#include "../src/puro.hpp"
#include <iomanip>

struct Grain
{
    Grain(int offset, int length, puro::DynamicBuffer<float>& sourceBuffer, int startIndex, float panning)
        : bounds(offset, length)
        , audioSrc(sourceBuffer)
        , audioSeq((float)startIndex, 1.0f)
        , envlSeq(puro::envl_halfcos_create_seq<float>(length))
        , panCoeffs(puro::pan_create_stereo(panning))
    {}

    puro::RelativeAlignment alignment;
    puro::DynamicBuffer<float>& audioSrc;
    puro::Sequence<float> audioSeq;
    puro::Sequence<float> envlSeq;
    puro::PanCoeffs<float, 2> panCoeffs;
};

struct Context
{
    std::vector<float> vec1;
    std::vector<float> vec2;
};

template <typename BufferType, typename ElementType, typename ContextType>
bool process_grain(BufferType dst, ElementType& grain, ContextType& context)
{
    std::tie(dst, grain.bounds) = puro::crop_buffer_and_advance_bounds(dst, grain.bounds);
	
    auto tmp1 = puro::wrap_vector<BufferType> (context.vec1, dst.length());
    tmp1 = puro::interp_crop_buffer(tmp1, grain.audioSrc.length(), grain.audioSeq, 1);
    std::tie(tmp1, grain.audioSeq) = puro::buffer_interp1_fill(tmp1, grain.audioSrc, grain.audioSeq);

    auto tmp2 = puro::wrap_vector<BufferType> (context.vec2, tmp1.length());

    puro::pan_apply(tmp2, tmp1, grain.panCoeffs);

    using MonoBufferType = puro::Buffer<typename BufferType::value_type, 1>;
    MonoBufferType envelope = puro::wrap_vector<MonoBufferType> (context.vec1, tmp2.length());
    grain.envlSeq = puro::envl_halfcos_fill(envelope, grain.envlSeq);

    BufferType output = puro::trimmed_length(dst, envelope.length());
    puro::multiply_add(output, tmp2, envelope);

    return (grain.bounds.remaining <= 0) || (output.length() != dst.length());
}

int main()
{
    using BufferType = puro::Buffer<float, 2>;
    std::vector<float> vec;
    BufferType output = puro::wrap_vector<BufferType>(vec, 256);

    Context context;

    puro::Timer<int> timer (0);

    AlignedPool<Grain> pool;
    pool.elements.reserve(16);

    std::vector<float> audioFileData;
    auto audioFileBuffer = puro::fit_vector_into_dynamic_buffer<puro::DynamicBuffer<float>> (audioFileData, 2, 2000);

    //puro::linspace_fill(audioFileBuffer, 0.0f, 1.0f);
    puro::noise_fill(audioFileBuffer);

    //GaussianParameter<float, int> intervalParameter(40, 0.001f);
    //GaussianParameter<float, int> durationParameter(10, 0.001f);
    //GaussianParameter<float, int> materialOffsetParameter(100, 3);

    const int blockSize = 32;

    for (int i=0; i<output.length(); i+=blockSize)
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
        while (n = timer.advance(n))
        {
            //const int interval = intervalParameter.get();
            //const int duration = std::max(durationParameter.get(), 5);
            //const int materialOffset = std::max(materialOffsetParameter.get(), 0);
            const int interval = 100;
            const int duration = 80;
            const int materialOffset = 1980;
            const float panning = 0.9f;

            timer.interval = interval;
            auto it = pool.push(Grain(blockSize - n, duration, audioFileBuffer, materialOffset, panning));

            if (it.isValid())
            {
                if (process_grain(buffer, it.get(), context))
                    pool.pop(it);
            }
        }
    }

    for (int i=0; i < output.length(); i++)
    {
        std::cout << std::setprecision(4) << std::fixed;
        std::cout << i << ":\t" << output(0, i) << "\t" << output(1, i) << std::endl;;
    }

    return 0;
}
