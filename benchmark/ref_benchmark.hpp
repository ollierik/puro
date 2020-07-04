#pragma once

#include "../../benchmark/include/benchmark/benchmark.h"

#ifdef _WIN32
#pragma comment ( lib, "Shlwapi.lib" )
#ifdef _DEBUG
#pragma comment ( lib, "benchmarkd.lib" )
#else
#pragma comment ( lib, "benchmark.lib" )
#endif
#endif

#define PURO_BUFFER_WRAP_VECTOR_RESIZING 0
#include "../src/puro.hpp"

struct Grain
{
    Grain(int offset, int length, int startIndex, float panning)
        : alignment(offset, length)
        , audioSequence((float)startIndex, 1.0f)
        , envelopeSequence(puro::content_envelope_halfcos_create_seq<float>(length))
        , panCoeffs(puro::pan_create_stereo(panning))
    {}

    puro::RelativeAlignment alignment;
    puro::Sequence<float> audioSequence;
    puro::Sequence<float> envelopeSequence;
    puro::PanCoeffs<float, 2> panCoeffs;
};

struct Context
{
    std::vector<float> vec1;
    std::vector<float> vec2;
};

template <typename BufferType, typename ElementType, typename ContextType, typename SourceBufferType>
bool process_grain(BufferType dst, ElementType& grain, ContextType& context, SourceBufferType source)
{
    using MonoBufferType = puro::Buffer<typename BufferType::value_type, 1>;

    puro::alignment_advance_and_crop_buffer(dst, grain.alignment);

    if (dst.isInvalid())
        return true;

    auto audio = puro::buffer_wrap_vector<SourceBufferType> (context.vec1, dst.length());
    puro::content_interpolation_crop_buffer(audio, source.length(), grain.audioSequence, 1);
    puro::content_interpolation1_fill(audio, source, grain.audioSequence);

    MonoBufferType envelope = puro::buffer_wrap_vector<MonoBufferType> (context.vec2, audio.length());
    puro::content_envelope_halfcos_fill(envelope, grain.envelopeSequence);

    puro::content_multiply_inplace(audio, envelope);

    BufferType output (dst);
    puro::buffer_trim_length(dst, audio.length());

    puro::content_pan_apply_and_add(output, audio, grain.panCoeffs);

    return (grain.alignment.remaining <= 0) || (output.length() != dst.length());
}


static void benchmark_ref(benchmark::State& state)
{
    const int blockSize = 512;
    const int numIterations = 100;

    using BufferType = puro::Buffer<float, 2>;
    std::vector<float> vec;
    BufferType buffer = puro::buffer_wrap_vector<BufferType, float, true>(vec, blockSize);

    Context context;
    context.vec1.resize(blockSize*4, 0);
    context.vec2.resize(blockSize*4, 0);

    puro::Timer<int> timer (0);

    puro::AlignedPool<Grain> pool;
    pool.elements.reserve(16);

    std::vector<float> sourceData;
    auto source = puro::buffer_wrap_vector<BufferType, float, true> (sourceData, blockSize);
    puro::noise_fill(source);

    for (auto _ : state)
    {
        for (int i = 0; i < numIterations; ++i)
        {
            puro::buffer_clear(buffer);

            for (auto&& it : pool)
            {
                if (process_grain(buffer, it.get(), context, source))
                {
                    pool.pop(it);
                }
            }

            int n = blockSize;
            while (n = timer.advance(n))
            {
                const int interval = 100;
                const int duration = (300 * 44100) / 1000;
                const int materialOffset = 10;
                const float panning = 0.0f;

                timer.interval = interval;
                auto it = pool.push(Grain(blockSize - n, duration, materialOffset, panning));

                if (it.isValid())
                {
                    if (process_grain(buffer, it.get(), context, source))
                        pool.pop(it);
                }
            }
        }
    } // BENCHMARK
}


BENCHMARK(benchmark_ref)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();