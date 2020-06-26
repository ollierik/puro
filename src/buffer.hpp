#pragma once

#include <array>

#ifndef PURO_BUFFER_MAX_CHANNELS
    // Maximum number of channels a Buffer object can hold. Unset the macro before including if surround formats needed.
    #define PURO_BUFFER_MAX_CHANNELS 2
#endif

/** Simple wrapper around audio buffer data with helper functions for accessing and debug checks. */
template <class FloatType>
class Buffer
{
private:

    int numChannels;
    int numSamples;
    std::array<FloatType*, PURO_BUFFER_MAX_CHANNELS> channels;

public:

    //////////////////

    Buffer() : numChannels(0), numSamples(0) {} // invalid Buffer

    Buffer (int numChannels, int numSamples)
        : numChannels(numChannels), numSamples(numSamples)
    {}

    Buffer (int numChannels,
        int numSamples,
        FloatType* channelData)
        : numChannels(numChannels), numSamples(numSamples)
    {
        for (int ch=0; ch<numChannels; ++ch)
            channels[ch] = &channelData[ch];
    }

    /** Create a buffer with given shape, with channels fitted into the provided vector.
        If vector can't fit the created buffer, it will be resized. */
    Buffer (int numChannels, int numSamples, std::vector<FloatType>& vector)
        : numChannels(numChannels), numSamples(numSamples)
    {
        // resize if needed
        if (vector.size() < numSamples * numChannels)
        {
            vector.resize(numSamples * numChannels);
        }

        for (int ch = 0; ch < numChannels; ++ch)
            channels[ch] = &vector[ch * numSamples];
    }

    /** Create a buffer that has same shape as the provided buffer, with channels fitted into the provided vector.
        If vector can't fit the created buffer, it will be resized. */
    Buffer (Buffer<FloatType>& other, std::vector<FloatType>& vector)
        : numChannels(other.numChannels), numSamples(other.numSamples)
    {
        errorif(other.numChannels > PURO_BUFFER_MAX_CHANNELS, "TODO implement this in a more general way");

        // resize if needed
        if (vector.size() < numSamples * numChannels)
        {
            vector.resize(numSamples * numChannels);
        }

        for (int ch = 0; ch < numChannels; ++ch)
            channels[ch] = &vector[ch * numSamples];
    }

    FloatType* channel(int ch) const
    {
        errorif(ch < 0 || ch > numChannels, "channel out of range");
        errorif(ch >= PURO_BUFFER_MAX_CHANNELS, "requested channel above maxmimum");
        return channels[ch];
    }

    bool isInvalid() const { return numChannels == 0 || numSamples == 0; }

    int getNumChannels() { return numChannels; };
    int size() const { return numSamples; };

    const std::pair<int, int> shape() const { return { numChannels, numSamples }; };

    void trimBegin(int offset)
    {
        errorif(offset < 0 || offset > numSamples, "offset out of bounds");

        numSamples -= offset;

        for (int ch=0; ch<numChannels; ++ch)
            channels[ch] = &channels[ch][offset];
    }

    void trimLength(int newLength)
    {
        errorif(newLength < 0 || newLength > numSamples, "new length out of bounds");

        numSamples = newLength;
    }

    void trimLengthToMatch(Buffer<FloatType>& other)
    {
        if (other.numSamples < numSamples)
        {
            numSamples = other.numSamples;
        }
    }

    bool dimensionsMatch(const Buffer<FloatType>& other)
    {
        return (size() == other.size() && getNumChannels() == other.getNumChannels());
    }
};



namespace ops
{
    enum class Type { replace, add };

    template <typename FloatType, class SourceType>
    static void buffer_fill_from_source(Buffer<FloatType>& dst, SourceType& source)
    {
        source.next(Type::replace, dst);
    }

    template <typename FloatType>
    static void buffer_multiply_add(Buffer<FloatType>& dst, const Buffer<FloatType>& src1, const Buffer<FloatType>& src2)
    {
        errorif(! (dst.shape() == src1.shape()), "dst and src1 buffer dimensions don't match");
        errorif(! (dst.shape() == src2.shape()), "dst and src2 buffer dimensions don't match");

        for (int ch=0; ch<dst.getNumChannels(); ++ch)
        {
            math::multiply_add<FloatType>(dst.channel(ch), src1.channel(ch), src2.channel(ch), dst.size());
        }
    }
};
