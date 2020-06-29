#pragma once

namespace puro {

/** A Wrapper around audio buffer data with helper functions for accessing and debug checks. Does not own the data. */
template <class FloatType, int numberOfChannels>
struct Buffer
{
    // member fields
    int numSamples;
    std::array<FloatType*, numberOfChannels> channelPtrs;

    // template arg broadcasts
    typedef FloatType value_type;
    static constexpr int num_channels = numberOfChannels;

    // getters
    bool isInvalid() const { return numSamples <= 0; }
    int size() const { return numSamples; };
    int getNumChannels() const { return num_channels; } // some more advanced class may want to redefine this

    FloatType& operator() (int ch, int i)
    {
        errorif(ch < 0 || ch >= num_channels, "channel out of range");
        errorif(i < 0 || i >= numSamples, "sample index out of range");
        return channelPtrs[ch][i];
    }

    FloatType* channel(int ch) const
    {
        errorif(ch < 0 || ch >= num_channels, "channel out of range");
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
        for (int ch = 0; ch < num_channels; ++ch)
            channelPtrs[ch] = &data[ch * numSamples];
    }

    /** Buffer from raw allocated memory. Provided array should contain the per-channel pointers. */
    Buffer (int numSamples, std::array<FloatType, numberOfChannels> data)
        : numSamples(numSamples)
    {
        for (int ch = 0; ch < data.size(); ++ch)
            channelPtrs[ch] = &data[ch];
    }
};


#ifndef PURO_DYNAMIC_BUFFER_MAX_CHANNELS
#define PURO_DYNAMIC_BUFFER_MAX_CHANNELS 8
#endif PURO_DYNAMIC_BUFFER_MAX_CHANNELS

/** Dynamic wrapper around audio buffer data with resizeable channel count. Does not own the data. */
template <class FloatType, int maxNumberOfChannels = PURO_DYNAMIC_BUFFER_MAX_CHANNELS>
struct DynamicBuffer
{
    // member fields
    int numSamples;
    int numChannels;
    std::array<FloatType*, maxNumberOfChannels> channelPtrs;

    // template arg broadcasts
    typedef FloatType value_type;

    // getters
    bool isInvalid() const { return numSamples <= 0 || numChannels <= 0; }
    int size() const { return numSamples; };
    int getNumChannels() const { return numChannels; } // some more advanced class may want to redefine this

    FloatType& operator() (int ch, int i)
    {
        errorif(ch < 0 || ch >= numChannels, "channel out of range");
        errorif(i < 0 || i >= numSamples, "sample index out of range");
        return channelPtrs[ch][i];
    }

    FloatType* channel(int ch) const
    {
        errorif(ch < 0 || ch >= numChannels, "channel out of range");
        return channelPtrs[ch];
    }

    // constructors

    DynamicBuffer() : numChannels(0), numSamples(0) {} // invalid Buffer

    DynamicBuffer (int numChannels, int numSamples)
        : numChannels(numChannels), numSamples(numSamples)
    {}

    /** Buffer from raw allocated memory.
        Provided data is expected to be able to hold (numSamples * numChannels) of data */
    DynamicBuffer (int numChannels, int numSamples, FloatType* data)
        : numChannels(numChannels), numSamples(numSamples)
    {
        for (int ch = 0; ch < numChannels; ++ch)
            channelPtrs[ch] = &data[ch * numSamples];
    }
};


////////////////////////////////
// Buffer operations
////////////////////////////////

template <typename BufferType>
BufferType trimmed_begin(BufferType buffer, int offset)
{
    errorif(offset < 0 || offset > buffer.numSamples, "offset out of bounds");

    buffer.numSamples -= offset;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
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

template <typename BufferType>
BufferType slice(BufferType buffer, int offset, int length)
{
    errorif(offset > buffer.numSamples, "slice offset greater than number of samples available");
    errorif(length < 0 || length > (offset + buffer.numSamples), "slice length out of bounds");

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.channelPtrs[ch] = &buffer.channelPtrs[ch][offset];

    buffer.numSamples = length;
    return buffer;
}

/** Create a Buffer with the data laid out into the provided vector.
    Resize the vector if needed. Number of channels is deducted from the template args. */
template <typename BufferType, typename FloatType>
BufferType wrap_vector(std::vector<FloatType>& vector, int numSamples)
{
    const int totLength = BufferType::num_channels * numSamples;

    // resize if needed
    if (vector.size() < totLength)
        vector.resize(totLength);

    return BufferType(numSamples, vector.data());
}

template <typename BufferType, typename FloatType>
BufferType fit_vector_into_dynamic_buffer(std::vector<FloatType>& vector, int numChannels, int numSamples)
{
    const int totLength = numChannels * numSamples;

    // resize if needed
    if (vector.size() < totLength)
        vector.resize(totLength);

    return BufferType(numChannels, numSamples, vector.data());
}

template <typename BufferType>
BufferType multiply_add(BufferType dst, const BufferType src1, const BufferType src2)
{
    errorif(!(dst.size() == src1.size()), "dst and src1 buffer lengths don't match");
    errorif(!(dst.size() == src2.size()), "dst and src2 buffer lengths don't match");

    for (int ch = 0; ch < dst.getNumChannels(); ++ch)
    {
        math::multiply_add(dst.channel(ch), src1.channel(ch), src2.channel(ch), dst.size());
    }
    return dst;
}


} // namespace puro
