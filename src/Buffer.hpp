#pragma once

/** Simple wrapper around audio buffer data with helper functions for accessing and debug checks. */
template <class FloatType>
struct Buffer
{
    std::array<FloatType*, 2> channels;
    int numChannels;
    int numSamples;

    //////////////////

    Buffer(int numChannels, int numSamples)
        : numChannels(numChannels) , numSamples(numSamples)
    {}

    Buffer(const std::array<FloatType*, 2>& channels, int numChannels, int numSamples)
        : channels(channels) , numChannels(numChannels), numSamples(numSamples)
    {}

    /** Construct a buffer that can accomodate provided buffer, fitted into the provided vector
        If vector can't fit the created buffer, it will be resized. */

    Buffer<FloatType> (Buffer<FloatType>& other, std::vector<FloatType>& vector)
        : numChannels(other.numChannels), numSamples(other.numSamples)
    {
        stopif(other.numChannels > 2, "TODO fix this");

        // resize if needed
        if (vector.size() < numSamples * numChannels)
        {
            vector.resize(numSamples * numChannels);
        }

        for (int ch=0; ch<numChannels; ++ch)
            channels[ch] = &vector[ch * numSamples];
    }

    void trimBegin(int offset)
    {
        stopif(offset < 0 || offset > numSamples, "offset out of bounds");

        numSamples -= offset;

        for (int ch=0; ch<numChannels; ++ch)
            channels[ch] = &channels[ch][offset];
    }

    void trimLength(int newLength)
    {
        stopif(newLength < 0 || newLength > numSamples, "new length out of bounds");

        numSamples = newLength;
    }

    void trimLengthToMatch(Buffer<FloatType>& other)
    {
        if (other.numSamples < numSamples)
        {
            numSamples = other.numSamples;
        }
    }

};

struct SourceOperations
{
    enum class Type { replace, add };

    template <typename FloatType, class SourceType>
    static void replace(Buffer<FloatType>& dst, SourceType& source)
    {
        source.next(dst, Type::replace);
    }

    template <typename FloatType>
    static void multiplyAdd(Buffer<FloatType>& dst, const Buffer<FloatType>& src1, const Buffer<FloatType>& src2)
    {
        stopif(dst.numChannels != src1.numChannels, "Number of channels doesn't match");
        stopif(dst.numChannels != src2.numChannels, "Number of channels doesn't match");
        stopif(dst.numSamples != src1.numSamples, "Number of samples doesn't match");
        stopif(dst.numSamples != src2.numSamples, "Number of samples doesn't match");

        for (int ch=0; ch<dst.numChannels; ++ch)
        {
            Math::multiplyAdd(dst.channels[ch], src1.channels[ch], src2.channels[ch], dst.numSamples);
        }
    }
};
