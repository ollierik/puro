#include <iostream>
#include <vector>

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

    using Buffer = ConstantBuffer<float, 32>;
    using Envelope = EnvelopeTemplate<float>;
    using AudioSource = NoiseSource<float>;
    using Grain = GrainTemplate<float, AudioSource, Envelope>;
    using Pool = FixedPool<Grain, 5>;
    using Controller = ControllerTemplate<Grain, AudioSource, Envelope>;
    using Engine = EngineTemplate<float, Buffer, Grain, Pool, Controller>;

    Controller scheduler;
    Engine engine(scheduler);

    const int blockSize = 32;

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


//#include "godbolt/godbolt_for_expansion.hpp"
