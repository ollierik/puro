#if 1

//#include "PuroHeader.h"
#include "puro.hpp"

int main()
{
    const int n = 512*2;
    std::vector<float> output (n, 0.0f);

    //using BlockSizeParameter = ConstIntParameter<32>;

    using BlockSizeParameter = IntParameter;

    using Envelope = EnvelopeTemplate<float>;
    using AudioSource = NoiseSource<float>;
    using Grain = GrainTemplate<float, AudioSource, Envelope>;
    using Pool = FixedPool<Grain, 5>;
    using Controller = ControllerTemplate<Grain, AudioSource, Envelope>;
    using Engine = EngineTemplate<float, BlockSizeParameter, Grain, Pool, Controller>;

    BlockSizeParameter blockSize(32);
    Controller controller;

    Engine engine(blockSize, controller);

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

#else
#include "godbolt/template_specialisation.hpp"
#endif
