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

#define POOL_SIZE 128

static void benchmark_gaussian_cached(benchmark::State& state)
{
    std::mt19937 gen;
    std::normal_distribution<float> dist;

    float sum = 0;

    for (auto _ : state)
    {
        const float random = dist(gen);
        sum += random;
    }
}

static void benchmark_mersenne(benchmark::State& state)
{
    std::mt19937 gen;
    std::uniform_int_distribution<int> dist (-1000, 1000);

    float sum = 0;
    for (auto _ : state)
        sum += dist(gen);
}

static void benchmark_lin(benchmark::State& state)
{
    std::minstd_rand gen;
    std::uniform_int_distribution<short> dist (std::numeric_limits<short>::min(), std::numeric_limits<short>::max());

    float sum = 0;
    for (auto _ : state)
        sum += dist(gen);
}

static void benchmark_subst(benchmark::State& state)
{
    std::ranlux24 gen;
    std::uniform_int_distribution<int> dist (-1000, 1000);

    float sum = 0;
    for (auto _ : state)
        sum += dist(gen);
}

static void benchmark_centlimit(benchmark::State& state)
{
    std::mt19937 gen;
    std::uniform_real_distribution<float> dist (-1, 1);

    float sum = 0;
    for (auto _ : state)
    {
        const float r = dist(gen) + dist(gen) + dist(gen);
        sum += r;
    }
}

static void benchmark_centlimit_int(benchmark::State& state)
{
    const int max = std::numeric_limits<int>::max() / 3;
    const float fdiv = 1.0f / (float)std::numeric_limits<int>::max();
    std::mt19937 gen;
    std::uniform_int_distribution<int> dist (-max, max);

    float sum = 0;
    for (auto _ : state)
    {
        const int r = dist(gen) + dist(gen) + dist(gen);
        const float f = (float)r * fdiv;
        sum += f;
    }
}

static void benchmark_centlimit_short(benchmark::State& state)
{
    constexpr int smin = std::numeric_limits<short>::min();
    constexpr int smax = std::numeric_limits<short>::max();
    constexpr float fdiv = 1.0f / (3.0f * static_cast<float>(smax));

    std::mt19937 gen;

    float sum = 0;
    for (auto _ : state)
    {
        const std::uniform_int_distribution<short> dist (smin, smax);
        const int r = dist(gen) + dist(gen) + dist(gen);
        const float f = (float)r * fdiv;
        sum += f;
    }
}

static void benchmark_centlimit_short_intcast(benchmark::State& state)
{
    constexpr int smin = std::numeric_limits<short>::min();
    constexpr int smax = std::numeric_limits<short>::max();
    constexpr float fdiv = 1.0f / (3.0f * static_cast<float>(smax));

    std::mt19937 gen;

    float sum = 0;
    for (auto _ : state)
    {
        const std::uniform_int_distribution<short> dist (smin, smax);
        const float f = static_cast<float>(static_cast<int>(dist(gen)) + static_cast<int>(dist(gen)) + static_cast<int>(dist(gen))) * fdiv;
        sum += f;
    }
}

static void benchmark_centlimit_short_floatcast(benchmark::State& state)
{
    constexpr int smin = std::numeric_limits<short>::min();
    constexpr int smax = std::numeric_limits<short>::max();
    constexpr float fdiv = 1.0f / (3.0f * static_cast<float>(smax));

    std::mt19937 gen;

    float sum = 0;
    for (auto _ : state)
    {
        const std::uniform_int_distribution<short> dist (smin, smax);
        const float f = (static_cast<float>(dist(gen)) + static_cast<float>(dist(gen)) + static_cast<float>(dist(gen))) * fdiv;
        sum += f;
    }
}


/*
BENCHMARK(benchmark_mersenne);
BENCHMARK(benchmark_lin);
BENCHMARK(benchmark_subst);

BENCHMARK(benchmark_centlimit);
BENCHMARK(benchmark_centlimit_int);
*/
BENCHMARK(benchmark_centlimit_short);
BENCHMARK(benchmark_centlimit_short_intcast);
BENCHMARK(benchmark_centlimit_short_floatcast);
BENCHMARK(benchmark_centlimit_short_intcast);
BENCHMARK(benchmark_centlimit_short_floatcast);

BENCHMARK_MAIN();