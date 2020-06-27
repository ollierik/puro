#pragma once

#include <array>

#ifndef PURO_BUFFER_MAX_CHANNELS
    // Maximum number of channels a Buffer object can hold. Unset the macro before including if surround formats needed.
    #define PURO_BUFFER_MAX_CHANNELS 2
#endif

/** Simple wrapper around audio buffer data with helper functions for accessing and debug checks. */
template <class FloatType, int numChannels>
class Buffer
{
private:

    //int numChannels;
    int numSamples;
    std::array<FloatType*, numChannels> channels;

public:

    typedef FloatType value_type;

    //////////////////

    Buffer() : numSamples(0) {} // invalid Buffer

    Buffer (int numSamples)
        : numSamples(numSamples)
    {}

    Buffer (int numSamples, FloatType* channelData)
        : numSamples(numSamples)
    {
        for (int ch=0; ch<numChannels; ++ch)
            channels[ch] = &channelData[ch];
    }

    /** Create a buffer with given shape, with channels fitted into the provided vector.
        If vector can't fit the created buffer, it will be resized. */
    Buffer (int numSamples, std::vector<FloatType>& vector)
        : numSamples(numSamples)
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
    Buffer (Buffer& other, std::vector<FloatType>& vector)
        : numSamples(other.numSamples)
    {
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

    void trimLengthToMatch(Buffer& other)
    {
        if (other.numSamples < numSamples)
        {
            numSamples = other.numSamples;
        }
    }

    bool dimensionsMatch(const Buffer& other)
    {
        return (size() == other.size() && getNumChannels() == other.getNumChannels());
    }
};



namespace ops
{
    enum class Type { replace, add };

    template <typename BufferType, class SourceType>
    static void buffer_fill_from_source(BufferType& dst, SourceType& source)
    {
        source.next(Type::replace, dst);
    }

    template <typename BufferType>
    static void buffer_multiply_add(BufferType& dst, const BufferType& src1, const BufferType& src2)
    {
        errorif(! (dst.size() == src1.size()), "dst and src1 buffer lengths don't match");
        errorif(! (dst.size() == src2.size()), "dst and src2 buffer lengths don't match");

        for (int ch=0; ch<dst.getNumChannels(); ++ch)
        {
            math::multiply_add(dst.channel(ch), src1.channel(ch), src2.channel(ch), dst.size());
        }
    }
};
