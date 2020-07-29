#pragma once

namespace puro {

/** A Wrapper around audio buffer data with helper functions for accessing and debug checks. Does not own the data. */
template <class FloatType, int numberOfChannels>
struct Buffer
{
    // template arg broadcasts
    typedef FloatType value_type;
    static constexpr int num_channels = numberOfChannels;
    
    // member fields
    int numSamples;
    std::array<FloatType*, numberOfChannels> channelPtrs;

    // getters
    bool isInvalid() const noexcept { return numSamples <= 0; }
    int length() const noexcept { return numSamples; }
    constexpr int getNumChannels() const noexcept { return num_channels; } // some more advanced class may want to redefine this

    FloatType* channel(int ch) const noexcept
    {
        errorif(ch < 0 || ch >= num_channels, "channel out of range");
        return channelPtrs[ch];
    }

    // constructors

    Buffer() : numSamples(0) {} // invalid Buffer

    Buffer (int numSamples) noexcept
        : numSamples(numSamples)
    {}

    /** Buffer from raw allocated memory.
        Provided data is expected to be able to hold (numSamples * numChannels) of data */
    Buffer (int numSamples, FloatType* data) noexcept
        : numSamples(numSamples)
    {
        for (int ch = 0; ch < num_channels; ++ch)
            channelPtrs[ch] = &data[ch * numSamples];
    }

    /** Buffer from raw allocated memory. Provided array should contain the per-channel pointers. */
    Buffer (int numSamples, std::array<FloatType, numberOfChannels> data) noexcept
        : numSamples(numSamples)
    {
        for (int ch = 0; ch < data.size(); ++ch)
            channelPtrs[ch] = &data[ch];
    }
    
    /** Buffer from already allocated memory per channel.
     Provided data is expected to be able to hold numSamples of data per channel */
    Buffer (int numSamples, std::array<FloatType*, numberOfChannels> chPtrs) noexcept
    : numSamples(numSamples)
    {
        for (int ch = 0; ch < num_channels; ++ch)
            channelPtrs[ch] = chPtrs[ch];
    }
};

#ifndef PURO_DYNAMIC_BUFFER_MAX_CHANNELS
#define PURO_DYNAMIC_BUFFER_MAX_CHANNELS 8
#endif 

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
    int length() const { return numSamples; };
    int getNumChannels() const { return numChannels; } // some more advanced class may want to redefine this

    FloatType* channel(int ch) const
    {
        errorif(ch < 0 || ch >= numChannels, "channel out of range");
        return channelPtrs[ch];
    }

    // constructors

    DynamicBuffer() : numSamples(0), numChannels(0) {} // invalid Buffer

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
BufferType buffer_trim_begin(BufferType buffer, int offset) noexcept
{
    errorif(offset < 0 || offset > buffer.numSamples, "offset out of bounds");

    buffer.numSamples -= offset;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.channelPtrs[ch] = &buffer.channelPtrs[ch][offset];

    return buffer;
}


template <typename BufferType>
BufferType buffer_trim_length(BufferType buffer, int newLength) noexcept
{
    errorif(newLength > buffer.numSamples, "new length out of bounds");

    buffer.numSamples = math::max(newLength, 0);
    return buffer;
}

/** Slice a piece of buffer with given offset and length */
template <typename BufferType>
BufferType buffer_slice(BufferType buffer, int offset, int length) noexcept
{
    errorif(offset > buffer.numSamples, "slice offset greater than number of samples available");
    errorif(length < 0 || length > (offset + buffer.numSamples), "slice length out of bounds");

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.channelPtrs[ch] = &buffer.channelPtrs[ch][offset];

    buffer.numSamples = length;
    return buffer;
}
    
/** Split the given buffer into from index. The second buffer starts with index at zeroeth index. */
template <typename BufferType>
std::tuple<BufferType, BufferType> buffer_split(BufferType buffer, int index) noexcept
{
    errorif(index <= 0, "split is 0 or below");
    errorif(index >= buffer.numSamples, "split greater than number of samples available");
    
    BufferType pre = buffer_trim_length(buffer, index);
    BufferType post = buffer_trim_begin(buffer, index);

    return std::make_tuple(std::move(pre), std::move(post));
}

/** Create a Buffer with the data laid out into the provided vector.
    The vector may be resized if needed depending on template arg. Number of channels is deducted from the template args. */
template <typename BufferType, typename VectorType, bool resizeIfNeeded = PURO_BUFFER_WRAP_VECTOR_RESIZING>
BufferType buffer_wrap_vector(VectorType& vector, int numSamples) noexcept
{
    if (resizeIfNeeded)
    {
        const auto totLength = BufferType::num_channels * numSamples;

        if (vector.size() < totLength)
            vector.resize(totLength);
    }

    return BufferType(numSamples, vector.data());
}
    
/** Create a Buffer with the data laid out into the provided vector.
 The vector may be resized if needed depending on template arg. Number of channels is deducted from the template args. */
template <typename BufferType, typename VectorType, bool resizeIfNeeded = PURO_BUFFER_WRAP_VECTOR_RESIZING>
BufferType buffer_wrap_vector_per_channel(std::array<VectorType&, 2> vectors, int numSamples) noexcept
{
    if (resizeIfNeeded)
    {
        for (auto ch=0; ch<vectors.size(); ++ch)
        {
            if (vectors[ch].size() < numSamples)
                vectors[ch].resize(numSamples);
        }
    }
    
    BufferType buffer (numSamples);
    
    for (auto ch=0; ch<vectors.size(); ++ch)
    {
        buffer.channelPtr[ch] = vectors[ch].data();
    }
    
    return buffer;
}

template <typename ToBufferType, typename FromBufferType>
ToBufferType buffer_convert_to_type(FromBufferType src) noexcept
{
    ToBufferType dst (src.length());
    for (int ch=0; ch < dst.getNumChannels(); ++ch)
    {
        errorif (ch >= src.getNumChannels(), "trying to convert from less channels to a larger one");
        dst.channelPtrs[ch] = src.channelPtrs[ch];
    }
    return dst;
}

template <typename BufferType, typename FloatType>
BufferType fit_vector_into_dynamic_buffer(std::vector<FloatType>& vector, int numChannels, int numSamples) noexcept
{
    const int totLength = numChannels * numSamples;

    // resize if needed
    if ((int)vector.size() < totLength)
        vector.resize(totLength);

    return BufferType(numChannels, numSamples, vector.data());
}

template <typename BufferType, typename MultBufferType>
BufferType buffer_multiply_add(BufferType dst, const BufferType src1, const MultBufferType src2) noexcept
{
    errorif(!(dst.length() == src1.length()), "dst and src1 buffer lengths don't match");
    errorif(!(dst.length() == src2.length()), "dst and src2 buffer lengths don't match");

    // identical channel configs
    if (src1.getNumChannels() == src2.getNumChannels())
    {
        for (int ch = 0; ch < dst.getNumChannels(); ++ch)
        {
            math::multiply_add(dst.channel(ch), src1.channel(ch), src2.channel(ch), dst.length());
        }
    }
    // src2 is a mono buffer
    else if (src1.getNumChannels() > 1 && src2.getNumChannels() == 1)
    {
        for (int ch = 0; ch < dst.getNumChannels(); ++ch)
        {
            math::multiply_add(dst.channel(ch), src1.channel(ch), src2.channel(0), dst.length());
        }
    }
    else
    {
        errorif(true, "channel config not implemented");
    }
       
    return dst;
}
    
template <typename BufferType>
BufferType buffer_scale(BufferType dst, const typename BufferType::value_type value) noexcept
{
    for (int ch = 0; ch < dst.getNumChannels(); ++ch)
    {
        math::multiply(dst.channel(ch), value, dst.length());
    }
}

template <typename BufferType, typename MultBufferType>
BufferType buffer_multiply(BufferType dst, const MultBufferType src) noexcept
{
    errorif(dst.length() != src.length(), "dst and src buffer lengths don't match");

    // identical channel config
    if (dst.getNumChannels() == src.getNumChannels())
    {
        for (int ch = 0; ch < dst.getNumChannels(); ++ch)
        {
            math::multiply(dst.channel(ch), src.channel(ch), dst.length());
        }
    }
    // mono src, multichannel dst
    else if (dst.getNumChannels() != src.getNumChannels() && src.getNumChannels() == 1)
    {
        for (int ch = 0; ch < dst.getNumChannels(); ++ch)
        {
            math::multiply(dst.channel(ch), src.channel(0), dst.length());
        }
    }
       
    return dst;
}
    
template <typename BufferType, typename AddBufferType>
BufferType buffer_add(BufferType dst, const AddBufferType src) noexcept
{
    errorif(dst.length() != src.length(), "dst and src buffer lengths don't match");

    // identical channel config
    if (dst.getNumChannels() == src.getNumChannels())
    {
        for (int ch = 0; ch < dst.getNumChannels(); ++ch)
        {
            math::add(dst.channel(ch), src.channel(ch), dst.length());
        }
    }
    // mono src, multichannel dst
    else if (dst.getNumChannels() != src.getNumChannels() && src.getNumChannels() == 1)
    {
        for (int ch = 0; ch < dst.getNumChannels(); ++ch)
        {
            math::add(dst.channel(ch), src.channel(0), dst.length());
        }
    }
    
    return dst;
}
    
template <typename BufferType, typename SubstBufferType>
BufferType buffer_substract(BufferType dst, const SubstBufferType src) noexcept
{
    errorif(dst.length() != src.length(), "dst and src buffer lengths don't match");

    // identical channel config
    if (dst.getNumChannels() == src.getNumChannels())
    {
        for (int ch = 0; ch < dst.getNumChannels(); ++ch)
        {
            math::substract(dst.channel(ch), src.channel(ch), dst.length());
        }
    }
    // mono src, multichannel dst
    else if (dst.getNumChannels() != src.getNumChannels() && src.getNumChannels() == 1)
    {
        for (int ch = 0; ch < dst.getNumChannels(); ++ch)
        {
            math::substract(dst.channel(ch), src.channel(0), dst.length());
        }
    }
    
    return dst;
}

template <typename BufferType>
void buffer_clear(BufferType buffer) noexcept
{
    for (int ch=0; ch<buffer.getNumChannels(); ++ch)
    {
        math::set<typename BufferType::value_type>(buffer.channel(ch), 0, buffer.length());
    }
}

} // namespace puro
