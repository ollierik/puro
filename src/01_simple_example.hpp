#pragma once
#include "puro.hpp"

// Simple struct to demonstrate the audio grain
struct Grain
{
    Grain(int offset, int length)
        : range(offset, length)
    {}

    puro::Range range;
};

// Temporary (hopefully) pre-allocated audio buffers for processing
struct Context
{
    std::vector<float> vec1;
    std::vector<float> vec2;
};

template <typename BufferType, typename ElementType, typename ContextType>
bool process_grain(const BufferType& buffer, ElementType& grain, ContextType& context)
{
    // Create an write buffer via cropping the input buffer
    // grain.range contain information about the per-block offset needed, and for the remaining duration of the grain
	BufferType output = puro::range_crop_buffer(grain.range, buffer);
    const int numSamplesToWrite = output.size();
	
    // Use the temporary vector vec1 provided by the context, wrap it to a Buffer
    BufferType audioBuffer = puro::wrap_vector<BufferType>(context.vec1, output.size());

    // Fill the audioBuffer with noise
    puro::noise_fill(audioBuffer);

    // Use the temporary vector vec2 provided by the context, wrap it to a Buffer
    // We're requesting as many samples as we got to the audioBuffer
    BufferType envelopeBuffer = puro::wrap_vector<BufferType>(context.vec2, audioBuffer.size());

    // Fill envelopeBuffer with constant ones
    // Obviously this could be skipped, as it will result with multiplication with one,
    // but it is presented here for clarity
    puro::constant_fill(envelopeBuffer, 1);

    // Trim the length of the output buffer in case we have less material to work with than what was requested
    // Since we've filled the audio buffer with noise, this is redundant, but presented here for clarity
    output = puro::trimmed_length(output, envelopeBuffer.size());

    // Multiply audioBuffer and envelopeBuffer and add the results to output
    // These are done on per-vector basis for SIMD acceleration
    output = puro::multiply_add(output, audioBuffer, envelopeBuffer);

    // Advance the grain.ranges with the block size
    grain.range = puro::range_advance(grain.range, buffer.size());

    // Return true if grain has been depleted either by running out of material or duration
    return (grain.range.remaining <= 0) || (output.size() != numSamplesToWrite);
}

int main()
{
    // Place-holder audio output buffer for writing
    using BufferType = puro::Buffer<float, 1>;
    std::vector<float> vec;
    BufferType output = puro::wrap_vector<BufferType>(vec, 256);

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
        BufferType buffer (blockSize, &vec[i]);

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
        int n = blockSize; // n marks how many samples we have left for the block
        while (n = scheduler.tick(n))
        {
            // Add the new grain with alignment offset and a duration of 40
            auto it = pool.push(Grain(blockSize - n, 10));

            if (it.isValid())
            {
                // Process the added grain immediately, and remove it directly if it was depleted immediately
                if (process_grain(buffer, it.get(), context))
                    pool.pop(it);
            }
        }
    }

    // Print the output samples
    for (auto i=0; i<output.size(); ++i)
    {
        std::cout << i << ": " << output(0, i) << std::endl;
    }

    return 0;
}
