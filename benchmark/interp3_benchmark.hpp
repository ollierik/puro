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

//#define PURO_DEBUG 0
#define PURO_BUFFER_WRAP_VECTOR_RESIZING 0
#include "../src/puro.hpp"

struct Grain
{
    Grain(int offset, int length, int startIndex, float panning, double velocity, int sourceLength)
        : alignment({ offset, length })
        , readPos(startIndex)
        , readInc(velocity)
        , envelopePos(puro::envelope_hann_get_increment<float>(length))
        , envelopeInc(envelopePos)
        , panCoeffs(puro::pan_create_stereo(panning))
    {
        std::tie(alignment, readPos) = puro::interp_avoid_out_of_bounds_reads<3>(alignment, readPos, readInc, sourceLength);
    }

    puro::RelativeAlignment alignment;

    double readPos;
    const double readInc;

    float envelopePos;
    const float envelopeInc;

    puro::PanCoeffs<float, 2> panCoeffs;
};

struct Context
{
    std::vector<float> vec1;
    std::vector<float> vec2;
    puro::DynamicBuffer<float> source;
};

template <typename BufferType, typename ElementType, typename ContextType, typename SourceBufferType>
bool process_grain(BufferType dst, ElementType& grain, ContextType& context, SourceBufferType source)
{
    using MonoBufferType = puro::Buffer<typename BufferType::value_type, 1>;

    std::tie(dst, grain.alignment) = puro::alignment_advance_and_crop_buffer(dst, grain.alignment);

    if (dst.isInvalid())
        return true;

    auto audio = puro::buffer_wrap_vector<SourceBufferType> (context.vec1, dst.length());
    grain.readPos = puro::interp3_fill(audio, source, grain.readPos, grain.readInc);

    MonoBufferType envelope = puro::buffer_wrap_vector<MonoBufferType> (context.vec2, audio.length());
    grain.envelopePos = puro::envelope_halfcos_fill(envelope, grain.envelopePos, grain.envelopeInc);

    puro::buffer_multiply(audio, envelope);

    BufferType output = puro::buffer_trim_length(dst, audio.length());

    puro::content_pan_apply_and_add(output, audio, grain.panCoeffs);

    return (grain.alignment.remaining <= 0) || (output.length() != dst.length());
}

static void benchmark_interp3(benchmark::State& state)
{
    const int blockSize = 512;
    const int numIterations = 100;

    using BufferType = puro::Buffer<float, 2>;
    std::vector<float> vec;
    BufferType buffer = puro::buffer_wrap_vector<BufferType, float, true>(vec, blockSize);

    Context context;
    context.vec1.resize(blockSize * 4, 0);
    context.vec2.resize(blockSize * 4, 0);

    puro::Timer<int> timer (0);

    puro::AlignedPool<Grain> pool;
    pool.elements.reserve(16);

    std::vector<float> sourceData;
    auto source = puro::buffer_wrap_vector<BufferType, float, true> (sourceData, 44100*2);
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

            int n = buffer.length();
            while ((n = timer.advance(n)) > 0)
            {
                const int interval = 100;
                const int duration = (300 * 44100) / 1000;
                const int readpos = 10;
                const float panning = 0.0f;
                const float velocity = 1.15f;
                timer.interval = interval;

                auto it = pool.push(Grain(blockSize - n, duration, readpos, panning, velocity, source.length()));

                if (it.isValid())
                {
                    if (process_grain(buffer, it.get(), context, source))
                        pool.pop(it);
                }
            }
        }
    }
}

BENCHMARK(benchmark_interp3)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
