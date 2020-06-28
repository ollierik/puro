#pragma once

#include <array>

/** Simple wrapper around audio buffer data with helper functions for accessing and debug checks. */
template <class FloatType, int numberOfChannels>
struct Buffer
{
    // member fields
    int numSamples;
    std::array<FloatType*, numberOfChannels> channelPtrs;

    // template arg broadcasts
    typedef FloatType value_type;
    static constexpr int numChannels = numberOfChannels;

    // getters
    bool isInvalid() const { return numSamples <= 0; }
    int size() const { return numSamples; };
    int getNumChannels() const { return numChannels; } // some more advanced class may want to redefine this

    FloatType* channel(int ch) const
    {
        errorif(ch < 0 || ch > numChannels, "channel out of range");
        return channelPtrs[ch];
    }

    // constructors

    Buffer() : numSamples(0) {} // invalid Buffer

    Buffer (int numSamples)
        : numSamples(numSamples)
    {}

    /** Buffer from raw allocated memory.
        Provided data is expected to be able to hold (numSamples * numChannels) of data */
    Buffer (int numSamples, FloatType* data)
        : numSamples(numSamples)
    {
        for (int ch=0; ch < numChannels; ++ch)
            channelPtrs[ch] = &data[ch * numSamples];
    }

    /** Buffer from raw allocated memory. Provided array should contain the per-channel pointers. */
    Buffer (int numSamples, std::array<FloatType, numberOfChannels> data)
        : numSamples(numSamples)
    {
        for (int ch=0; ch < data.size(); ++ch)
            channelPtrs[ch] = &data[ch];
    }
};

/** Buffer operations
    These operations feature buffer size manipulations,
    and act as the glue between math operations and buffer structs. */
namespace bops
{
    template <typename BufferType>
    BufferType trimmed_begin(BufferType buffer, int offset)
    {
        errorif(offset < 0 || offset > buffer.numSamples, "offset out of bounds");

        buffer.numSamples -= offset;

        for (int ch=0; ch < buffer.getNumChannels(); ++ch)
            buffer.channelPtrs[ch] = &buffer.channelPtrs[ch][offset];

        return buffer;
    }


    template <typename BufferType>
    BufferType trimmed_length(BufferType buffer, int newLength)
    {
        errorif(newLength < 0 || newLength > buffer.numSamples, "new length out of bounds");

        buffer.numSamples = newLength;
        return buffer;
    }


    template <typename BufferType, typename FloatType>
    BufferType fit_vector(std::vector<FloatType>& vector, int numSamples)
    {
        const int totLength = BufferType::numChannels * numSamples;

        // resize if needed
        if (vector.size() < totLength)
            vector.resize(totLength);

        return BufferType(numSamples, vector.data());
    }


    enum class Type { replace, add };


    template <typename BufferType, class SourceType>
    BufferType filled_from_source(BufferType dst, SourceType& source)
    {
        return source.next(Type::replace, dst);
    }


    template <typename BufferType>
    BufferType multiply_add(BufferType dst, const BufferType src1, const BufferType src2)
    {
        errorif(! (dst.size() == src1.size()), "dst and src1 buffer lengths don't match");
        errorif(! (dst.size() == src2.size()), "dst and src2 buffer lengths don't match");

        for (int ch=0; ch < dst.numChannels; ++ch)
        {
            math::multiply_add(dst.channel(ch), src1.channel(ch), src2.channel(ch), dst.size());
        }

        return dst;
    }
};
