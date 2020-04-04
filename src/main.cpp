#include "PuroHeader.h"

// TODO
// - Multichannel
//      - Envelope max channels?
// - Random deviation parameters 

int main()
{
    const int n = 256;
    const int blockSize = 32;
    const int numChannels = 2;

    std::vector<float> left (n, 0.0f);
    std::vector<float> right (n, 0.0f);

    using Envelope = HannEnvelope<float>;
    using AudioSource = ConstSource<float, 1>;

    using Grain = GrainTemplate<float, AudioSource, Envelope>;
    using Wrapper = OffsetWrapper<float, Grain>;
    using Pool = FixedPool<Wrapper, 4>;

    using Engine = EngineTemplate<float, Grain, Pool, Wrapper>;
    using Controller = ControllerTemplate<Grain, AudioSource, Envelope, Engine>;

    Engine engine;
    Controller controller (engine);


    for (int i = 0; i < n - blockSize; i += blockSize)
    {
        Buffer<float> buffer ( { &left[i], &right[i] }, numChannels, blockSize);

        controller.advance(blockSize);
        engine.addNextOutput(buffer);
    }


    std::cout << "\n    OUTPUT\n----------\n";

    for (int i=0; i<n; i++)
    {
        std::cout << i << ":\t" << left[i] << "\t" << right[i] << std::endl;
    }

    return 0;
}
