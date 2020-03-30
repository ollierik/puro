#include <iostream>
#include <vector>

//#if 0

#include "Parameter.hpp"
#include "Envelope.hpp"
#include "AudioSource.hpp"
#include "Grain.hpp"
#include "Pool.hpp"
#include "Engine.hpp"
#include "Controller.hpp"


int main ()
{
    const int n = 512*2;
    std::vector<float> output (n, 0.0f);

    using BlockSizeType = ConstIntParameter<32>;
    static const BlockSizeType blockSize;

    using Envelope = EnvelopeTemplate<float>;
    using AudioSource = NoiseSource<float>;
    using Grain = GrainTemplate<float, AudioSource, Envelope>;
    using Pool = FixedPool<Grain, 5>;
    using Controller = ControllerTemplate<Grain, AudioSource, Envelope>;
    //using Engine = EngineTemplate<float, Buffer, Grain, Pool, Controller>;

    //using Engine = EngineTemplate<float, blockSize, Grain, Pool, Controller>;

    //BlockSize bs;

    Controller controller;
    EngineTemplate<float, BlockSizeType, blockSize, Grain, Pool, Controller> engine(controller);

    for (int i=0; i<n; i+=blockSize.getValue())
    {
        engine.tick(&output[i], blockSize.getValue());
    }

    std::cout << "\n    OUTPUT\n----------\n";

    for (int i=0; i<n; i++)
    {
        std::cout << i << ": " << output[i] << std::endl;
    }

    return 0;
}
//#endif

//#include "godbolt/type_deduction.hpp"
