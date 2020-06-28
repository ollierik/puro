#pragma once
#include "puro.hpp"

// Simple struct to demonstrate the audio grain
struct Grain
{
    Grain(int offset, int length)
        : ranges(offset, length)
    {}

    engine::Ranges ranges;
    ConstSource<float> source;
    ConstSource<float> envelope;
};

// Temporary (hopefully) pre-allocated audio buffers for processing
struct Context
{
    std::vector<float> temp1;
    std::vector<float> temp2;
};

template <typename BufferType, typename ElementType, typename ContextType>
bool process_grain(const BufferType& buffer, ElementType& grain, ContextType& context)
{
    // Create an write buffer via cropping the input buffer
    // grain.ranges contain information about the per-block offset needed, and for the remaining duration of the grain
	BufferType output = engine::ranges_crop_buffer(grain.ranges, buffer);
    const int numSamplesToWrite = output.size();
	
    // Use the temporary vector temp1 provided by the context, wrap it to a Buffer
    BufferType audioBuffer = bops::fit_vector<BufferType>(context.temp1, output.size());

    // Fill buffer contents from the grain.source
    // The source could decide to trim the buffer if it for example encountered the end of audio file
    audioBuffer = bops::filled_from_source(audioBuffer, grain.source);

    // Use the temporary vector temp2 provided by the context, wrap it to a Buffer
    // We're requesting as many samples as we got to the audioBuffer
    BufferType envelopeBuffer = bops::fit_vector<BufferType>(context.temp2, audioBuffer.size());

    // Fill buffer contents from the grain.envelope
    envelopeBuffer = bops::filled_from_source(envelopeBuffer, grain.envelope);

    // Trim the length of the output buffer in case we have less material to work with than what was requested
    output = bops::trimmed_length(output, envelopeBuffer.size());

    // Multiply audioBuffer and envelopeBuffer and add the results to output
    // These are done on per-vector basis for SIMD acceleration
    output = bops::multiply_add(output, audioBuffer, envelopeBuffer);

    // Advance the grain.ranges with the block size
    grain.ranges = engine::ranges_advance(grain.ranges, buffer.size());

    // Return true if grain has been depleted either by running out of material or duration
    return (grain.ranges.remaining <= 0) || (output.size() != numSamplesToWrite);
}

int main()
{
    // Place-holder audio output buffer for writing
    using BufferType = Buffer<float, 1>;
    std::vector<float> vec;
    BufferType output = bops::fit_vector<BufferType>(vec, 256);

    // Context to provide temporary allocated memory to grain ouput
    Context context;

    // Simple scheduler that creates a grain every 20th sample
    SimpleScheduler scheduler (20);

    // Pool of grains to use
    AlignedPool<Grain> pool;
    pool.elements.reserve(16);

    const int blockSize = 32;

    // Loop that mimics the workings of an audio thread
    for (int i=0; i<output.size(); i+=blockSize)
    {
        // Buffer that we should fill every block with output
        Buffer<float, 1> buffer (blockSize, &vec[i]);

        // Iterate through all grains in the pool
        for (auto&& it : pool)
        {
            // For each grain, call a user-defined function, no strings attached or hand held
            // If process_grain returns true, the grain has depleted and should be removed
            if (process_grain(buffer, it.get(), context))
            {
                pool.pop(it);
            }
        }

        // Add new grains based on the scheduler operation
        int n = blockSize; // n marks how many samples we have for the block
        while (n = scheduler.tick(n))
        {
            // Add the new grain with alignment offset and a duration of 40
            auto it = pool.push(Grain(blockSize - n, 40));

            if (it.isValid())
            {
                // Process the added grain immediately, and remove it directly if it was depleted immediately
                if (process_grain(buffer, it.get(), context))
                    pool.pop(it);
            }
        }
    }

    // Print the output samples
    for (int i=0; i<vec.size(); i++)
    {
        std::cout << i << ": " << vec[i] << std::endl;
    }

    return 0;
}
