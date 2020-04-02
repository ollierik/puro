#include "PuroHeader.h"

// TODO
// - Mechanism for changing audio file buffer on-the-fly
//      - Is this needed, tho?
// - Audio file reader and utility class
// - Parameters for controller randomness
// - Multichannel via Buffer
// - BlockSizeParam to its own type? Should represent channel config as well

int main()
{
    const int n = 512*2;
    std::vector<float> output (n, 0.0f);

    using Envelope = EnvelopeTemplate<float>;

    using AudioSource = AudioBufferSource<float>;

    using Grain = GrainTemplate<float, AudioSource, Envelope>;
    using Pool = FixedPool<Grain, 4>;
    using Controller = ControllerTemplate<Grain, AudioSource, Envelope>;
    using Engine = EngineTemplate<float, Grain, Pool, Controller>;


    std::vector<float> fileBuffer(1024, 0.0f);
    for (auto& f : fileBuffer)
    {
        //f = ((float)std::rand() / (float)RAND_MAX) * 2 - 1;
        f = 1.0f;
    }

    std::function<AudioSource()> audioSourceFactory = [&fileBuffer]()
    {
        std::cout << "create audio source" << std::endl;
        AudioSource as (fileBuffer, 0);
        return as;
    };

    Controller controller;

    controller.bindAudioSourceFactory(audioSourceFactory);

    Engine engine(controller);

    const int blockSize = 64;

    for (int i=0; i<n; i+=blockSize)
    {
        engine.tick(&output[i], blockSize);
    }

    std::cout << "\n    OUTPUT\n----------\n";

    for (int i=0; i<n; i++)
    {
        std::cout << i << ": " << output[i] << std::endl;
    }

    return 0;
}
