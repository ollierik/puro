#include "PuroHeader.h"

// TODO
// - Multichannel
//      - Envelope max channels?
// - Random deviation parameters 

int main()
{
    const int n = 512 * 8;
    std::vector<float> output (n, 0.0f);

    {
        using Envelope = HannEnvelope<float>;
        //using AudioSource = AudioBufferSource<float>;

        using AudioSource = ConstSource<float, 1>;

        using Grain = GrainTemplate<float, AudioSource, Envelope>;
        using Pool = FixedPool<Grain, 4>;
        using Controller = ControllerTemplate<Grain, AudioSource, Envelope>;
        using Engine = EngineTemplate<float, Grain, Pool, Controller>;

        Controller controller;

        /*
        std::vector<float> fileBuffer(1024, 0.0f);
        for (auto& f : fileBuffer)
        {
            //f = ((float)std::rand() / (float)RAND_MAX) * 2 - 1;
            f = 1.0f;
        }
        controller.audioSourceFactoryCallback = [&fileBuffer]()
        {
            std::cout << "create audio source" << std::endl;
            AudioSource as (fileBuffer, 0);
            return as;
        };
        */

        const int blockSize = 64;

        Engine engine(controller);

        for (int i = 0; i < output.size() - blockSize; i += blockSize)
        {
            engine.tick(&output[i], blockSize);
        }
    }

    std::cout << "\n    OUTPUT\n----------\n";

    for (int i=0; i<n; i++)
    {
        std::cout << i << ": " << output[i] << std::endl;
    }

    return 0;
}
