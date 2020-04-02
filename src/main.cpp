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

    double end()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds> (end - start).count();

        return (double)duration;

        //std::cout << "Timeit stopped, duration: " << duration << " (ns)" << std::endl;
    }

private:
    std::chrono::high_resolution_clock::time_point start;
};



#define BLOCK_SIZE 64
#define N_BLOCKS 1000
#define POOL_SIZE 2048

int argBlockSize;

void fixed_benchmark(std::vector<float>& output, std::vector<float>& fileBuffer)
{

    using BlockSizeParameter = ConstIntParameter<BLOCK_SIZE>;
    using Envelope = EnvelopeTemplate<float>;
    using AudioSource = AudioBufferSource<float>;

    using Grain = GrainTemplate<float, AudioSource, Envelope>;
    using Pool = FixedPool<Grain, POOL_SIZE>;
    using Controller = ControllerTemplate<Grain, AudioSource, Envelope>;
    using Engine = EngineTemplate<float, BlockSizeParameter, Grain, Pool, Controller>;

    std::function<AudioSource()> audioSourceFactory = [&fileBuffer]()
    {
        AudioSource as = AudioSource(fileBuffer, 0);
        return as;
    };

    BlockSizeParameter blockSize;
    Controller controller;

    controller.bindAudioSourceFactory(audioSourceFactory);

    Engine engine(blockSize, controller);

    for (int i=0; i<N_BLOCKS; i++)
    {
        engine.tick(&output[0], blockSize);
    }
}

void dynamic_benchmark(std::vector<float>& output, std::vector<float>& fileBuffer)
{
    using BlockSizeParameter = IntParameter;

    using Envelope = EnvelopeTemplate<float>;

    using AudioSource = AudioBufferSource<float>;

    using Grain = GrainTemplate<float, AudioSource, Envelope>;
    using Pool = DynamicPool<Grain, POOL_SIZE>;
    using Controller = ControllerTemplate<Grain, AudioSource, Envelope>;
    using Engine = EngineTemplate<float, BlockSizeParameter, Grain, Pool, Controller>;

    std::function<AudioSource()> audioSourceFactory = [&fileBuffer]()
    {
        AudioSource as = AudioSource(fileBuffer, 0);
        return as;
    };

    BlockSizeParameter blockSize(argBlockSize);
    Controller controller;

    controller.bindAudioSourceFactory(audioSourceFactory);

    Engine engine(blockSize, controller);
    engine.reserveBufferSize(blockSize);

    for (int i=0; i<N_BLOCKS; i++)
    {
        engine.tick(&output[0], blockSize);
    }

}

int main(int argc, char* argv[])
{
    argBlockSize = std::atoi(argv[1]);
    const int samplerate = 44100;

    std::vector<float> fileBuffer(44100, 0.0f);
    std::vector<float> output (argBlockSize, 0.0f);

    double dynamicDur = 1e30;

    for (int iter = 0; iter < 5; iter++)
    {
        for (auto& f : fileBuffer)
        {
            f = ((float)std::rand() / (float)RAND_MAX) * 2 - 1;
        }

        // DYNAMIC
        {
            TimeIt tm;
            dynamic_benchmark(output, fileBuffer);

            dynamicDur = std::min<double>(tm.end(), dynamicDur);

            float sum = 0;
            for (int i = 0; i < argBlockSize; i++)
            {
                sum += output[i];
            }
            std::cout << "(" << sum << ") ";
        }

        for (auto& f : output)
            f = 0;
    }

    std::cout << "\nBlock size: " << argBlockSize << std::endl;
    //std::cout << "Fixed: " << fixedDur << std::endl;
    std::cout << "Dynamic: " << dynamicDur << std::endl;

    return 0;
}
