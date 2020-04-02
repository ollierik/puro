#pragma once

template <typename FloatType>
class NoiseSource
{
public:
    int getNextOutput(FloatType* vec, int numSamples)
    {
        for (int i=0; i<numSamples; i++)
        {
            const FloatType sample = static_cast<FloatType> (std::rand()) / static_cast<FloatType> (RAND_MAX);
            vec[i] = sample;
        }

        return numSamples;
    }
};


template <typename FloatType>
class AudioBufferSource
{
public:
    AudioBufferSource(std::vector<FloatType>& fileBuffer, int startIndex)
        : buffer(fileBuffer)
        , index(startIndex)
    {
    }


    int getNextOutput(FloatType* vec, int numSamples)
    {
        const auto bufferSize = buffer.size();
        const auto numRemaining = (bufferSize < index + numSamples) ? bufferSize - index : numSamples;

        // if audio buffer is going to end before the number of requested samples
        if (numRemaining < numSamples)
        {
            for (int i=0; i<numRemaining; ++i)
            {
                vec[i] = buffer[index++];
            }
            return static_cast<int> (numRemaining);
        }
        else
        {
            for (int i=0; i<numSamples; i++)
            {
                vec[i] = buffer[index++];
            }

            return numSamples;
        }
    }

private:
    int index;
    std::vector<FloatType>& buffer;
};
