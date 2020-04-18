#include "PuroHeader.h"

// TODO
// - Random deviation parameters for granulator
// - ints to size_ts where appropriate

int main()
{
    const int n = 200;
    const int blockSize = 32;
    const int numChannels = 2;

    std::vector<float> left (n, 0.0f);
    std::vector<float> right (n, 0.0f);

    //using Envelope = SineEnvelope<float>;
    using AudioSource = AudioBufferSource<float>;
    using Envelope = ConstSource<float>;
    //using AudioSource = ConstSource<float>;
    using Interpolator = LinearInterpolator<float>;

    using Context = InterpolatingProcessorContext<float>;
    using Grain = InterpolatingProcessor<float, Context, AudioSource, Envelope, Interpolator>;
    using Sound = SoundObject<float, Grain, Context>;

    using Engine = SoundObjectEngine<float, Sound, Grain, Context>;
    using Controller = BufferedGranularController<float, Engine, Sound, AudioSource, Envelope, Interpolator>;

    Engine engine;
    Controller controller (engine);

    engine.pool.reserve(8);

    std::vector<float> fileVector;

    Buffer<float> fileBuffer (2, 1024, fileVector);

    for (int i=0; i<fileBuffer.size(); ++i)
    {
        /*
        const float r1 = ((float)std::rand() / (float)RAND_MAX) * 2 -1;
        fileBuffer.channel(0)[i] = r1;

        const float r2 = ((float)std::rand() / (float)RAND_MAX) * 2 -1;
        fileBuffer.channel(1)[i] = r2;
        */
        //fileBuffer.channel(0)[i] = (float)(i % 2);
        //fileBuffer.channel(1)[i] = (float)(i % 2);

        fileBuffer.channel(0)[i] = (float)(i*10 + 10);
        fileBuffer.channel(1)[i] = (float)(i*10 + 10);
    }

    controller.setAudioBuffer(fileBuffer);

    for (int i = 0; i < n - blockSize; i += blockSize)
    {
        Buffer<float> buffer ( { &left[i], &right[i] }, numChannels, blockSize);

        engine.addNextOutput(buffer);
        controller.advance(buffer);
    }


    std::cout << "\n    OUTPUT\n----------\n";

    for (int i=0; i<n; i++)
    {
        std::cout << i << ":\t" << left[i] << "\t" << right[i] << std::endl;
    }

    return 0;
}
