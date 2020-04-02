#include "PuroHeader.h"

// TODO
// - Mechanism for changing audio file buffer on-the-fly
//      - Is this needed, tho?
// - Audio file reader and utility class
// - Parameters for controller randomness
// - Multichannel via Buffer
// - BlockSizeParam to its own type? Should represent channel config as well

#include <chrono>

class TimeIt
{
public:
    TimeIt() : start(std::chrono::high_resolution_clock::now())
    {
    }

    void end()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds> (end - start).count();

        if (duration <= 0)
        {
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

            std::cout << "Timeit stopped, duration: " << duration << " (ns)" << std::endl;
        }
        else
        {
            std::cout << "Timeit stopped, duration: " << duration << " (ms)" << std::endl;
        }
    }

private:
    std::chrono::high_resolution_clock::time_point start;
};



#define BLOCK_SIZE 32

void fixed_benchmark(std::vector<float>& output, std::vector<float>& fileBuffer)
{

    using BlockSizeParameter = ConstIntParameter<BLOCK_SIZE>;
    using Envelope = EnvelopeTemplate<float>;
    using AudioSource = AudioBufferSource<float>;

    using Grain = GrainTemplate<float, AudioSource, Envelope>;
    using Pool = FixedPool<Grain, 1000>;
    using Controller = ControllerTemplate<Grain, AudioSource, Envelope>;
    using Engine = EngineTemplate<float, BlockSizeParameter, Grain, Pool, Controller>;

    std::function<AudioSource&&()> audioSourceFactory = [&fileBuffer]()
    {
        return std::move(AudioSource(fileBuffer, 0));
    };

    BlockSizeParameter blockSize;
    Controller controller;

    controller.bindAudioSourceFactory(audioSourceFactory);

    Engine engine(blockSize, controller);

    for (int i=0; i<output.size(); i+=blockSize)
    {
        engine.tick(&output[i], blockSize);
    }
}

void dynamic_benchmark(std::vector<float>& output, std::vector<float>& fileBuffer)
{
    using BlockSizeParameter = IntParameter;

    using Envelope = EnvelopeTemplate<float>;

    using AudioSource = AudioBufferSource<float>;

    using Grain = GrainTemplate<float, AudioSource, Envelope>;
    using Pool = DynamicPool<Grain, 1000>;
    using Controller = ControllerTemplate<Grain, AudioSource, Envelope>;
    using Engine = EngineTemplate<float, BlockSizeParameter, Grain, Pool, Controller>;

    std::function<AudioSource&&()> audioSourceFactory = [&fileBuffer]()
    {
        return std::move(AudioSource(fileBuffer, 0));
    };

    BlockSizeParameter blockSize(BLOCK_SIZE);
    Controller controller;

    controller.bindAudioSourceFactory(audioSourceFactory);

    Engine engine(blockSize, controller);
    engine.reserveBufferSize(BLOCK_SIZE);

    for (int i=0; i<output.size(); i+=blockSize)
    {
        engine.tick(&output[i], blockSize);
    }

}

int main()
{
    const int samplerate = 44100;
    const int n = samplerate*10;

    std::vector<float> fileBuffer(1024, 0.0f);
    for (auto& f : fileBuffer)
    {
        f = ((float)std::rand() / (float)RAND_MAX) * 2 - 1;
    }

    std::vector<float> output (n, 0.0f);

    for (int iter = 0; iter < 5; iter++)
    {
        // FIXED
        {
            std::cout << "\nFIXED:\n";
            TimeIt tm;
            fixed_benchmark(output, fileBuffer);
            tm.end();

            float sum = 0;
            for (int i = 0; i < n; i++)
            {
                sum += output[i];
            }
            std::cout << "    dummy: " << sum << std::endl;
        }

        std::cout << "asd\n";

        // DYNAMIC
        {
            std::cout << "\nDYNAMIC\n";
            TimeIt tm;
            dynamic_benchmark(output, fileBuffer);
            tm.end();
            float sum = 0;
            for (int i = 0; i < n; i++)
            {
                sum += output[i];
            }
            std::cout << "    dummy: " << sum << std::endl;
        }
    }

    return 0;
}
