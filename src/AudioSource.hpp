#pragma once

#include "structs.hpp"
#include <cstdlib>

template <typename FloatType>
class AudioSourceTemplate
{
public:
    virtual int getNextOutput(FloatType* vec, int numSamples) = 0;
};

template <typename FloatType>
class NoiseSource : public AudioSourceTemplate<FloatType>
{
public:
    int getNextOutput(FloatType* vec, int numSamples) override
    {
        for (int i=0; i<numSamples; i++)
        {
            const FloatType sample = static_cast<FloatType> (std::rand()) / static_cast<FloatType> (RAND_MAX);
            vec[i] = sample;
        }

        return numSamples;
    }
};

/*
template <typename FloatType, class BufferType>
class AudioFileSource : public AudioSourceTemplate<FloatType>
{
    AudioFileSource(Buffer& fileBuffer, int startIndex)
        : buffer(fileBuffer)
        , index(startIndex)
    {
    }

    int getNextOutput(FloatType* vec, int numSamples) override
    {
        const int bufferSize = buffer.size();
        const int n = (bufferSize < index + numSamples) ? bufferSize - index : numSamples;
        if (n < numSamples)
        {
            for (int i=0; i<n; ++i)
            {
                vec[i] = buffer[index++];
            }
            return n;
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

    int index = 0;
    BufferType& buffer;
};
*/
