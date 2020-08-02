#pragma once

namespace puro {

#ifndef PURO_BUFFER_DEFAULT_MAX_CHANNELS
    #define PURO_BUFFER_DEFAULT_MAX_CHANNELS 2
#endif

#ifndef PURO_BUFFER_WRAP_VECTOR_RESIZING
    #define PURO_BUFFER_WRAP_VECTOR_RESIZING true
#endif

namespace detail {

template <typename T, bool FixedChannels, bool FixedSamples, int NumChannels, int Length>
struct buffer_members {};

/** Dynamic channels, dynamic length */
template <typename T, int NumChannels, int Length>
struct buffer_members <T, false, false, NumChannels, Length>
{
    buffer_members() = default;
    buffer_members(int num_channels, int length)
        : num_channels(num_channels), length(length) {}
    buffer_members(int num_channels, int length, std::array<T, NumChannels> channelPtrs)
        : num_channels(num_channels), length(length), ptrs(channelPtrs) {}

    int num_channels = 0;
    int length = 0;
    std::array<T*, NumChannels> ptrs;
};

/** Dynamic channels, fixed length */
template <typename T, int NumChannels, int Length>
struct buffer_members <T, false, true, NumChannels, Length>
{
    buffer_members() = default;
    buffer_members(int num_channels)
        : num_channels(num_channels) {}
    buffer_members(int num_channels, std::array<T, NumChannels> channelPtrs)
        : num_channels(num_channels), ptrs(channelPtrs) {}

    int num_channels = 0;
    static constexpr int length = Length;
    std::array<T*, NumChannels> ptrs;
};

/** Fixed channels, dynamic length */
template <typename T, int NumChannels, int Length>
struct buffer_members <T, true, false, NumChannels, Length>
{
    buffer_members() = default;
    buffer_members(int length)
        : length(length) {}
    buffer_members(int length, std::array<T, NumChannels> channelPtrs)
        : length(length), ptrs(channelPtrs) {}

    static constexpr int num_channels = NumChannels;
    int length = 0;
    std::array<T*, NumChannels> ptrs;
};

/** Fixed channels, fixed length */
template <typename T, int NumChannels, int Length>
struct buffer_members <T, true, true, NumChannels, Length>
{
    buffer_members() = default;
    buffer_members(std::array<T, NumChannels> channelPtrs)
        : ptrs(channelPtrs) {}
    static constexpr int num_channels = NumChannels;
    static constexpr int length = Length;
    std::array<T*, NumChannels> ptrs;
};


struct not_implemented { not_implemented() = delete; };

} // namespace detail

constexpr int operator"" _maxchs (unsigned long long n)
{
    return -static_cast<int> (n);
};

template <typename T, int NumChannels=-PURO_BUFFER_DEFAULT_MAX_CHANNELS, int Length=-1>
struct buffer
{
    typedef T value_type;

    static constexpr bool has_fixed_channels = NumChannels > 0;
    static constexpr bool has_fixed_length= Length > 0;
    static constexpr int max_channels = NumChannels > 0 ? NumChannels : -NumChannels;

    using Members = detail::buffer_members<T, has_fixed_channels, has_fixed_length, max_channels, Length>;
    Members members;

    ///////////////////////// CONSTRUCTORS /////////////////////////

    buffer() = default;

    template <typename ...Args>
    buffer(Args... args) : members(args...) {}

    ///////////////////////// MEMBER FUNCTIONS /////////////////////////

    /** Get the constexpr number of channels for the buffer.
        To harness constexpr where applicable, this function is overloaded so that the variant to use has the signature:
        int num_channels();
    */
    static constexpr int num_channels(typename std::conditional<has_fixed_channels, void, detail::not_implemented>::type)
    {
        return Members::num_channels;
    }

    /** Get number of channels for the buffer.
        To harness constexpr where applicable, this function is overloaded so that the variant to use has the signature:
        int num_channels();
    */
    int num_channels(typename std::conditional<has_fixed_channels, detail::not_implemented, void>::type)
    {
        return members.num_channels;
    }

    /** Get constexpr length per channels for the buffer.
        To harness constexpr where applicable, this function is overloaded so that the variant to use has the signature:
        int length();
    */
    static constexpr int length(typename std::conditional<has_fixed_length, void, detail::not_implemented>::type)
    {
        return Members::length;
    }

    /** Get length per channels for the buffer.
        To harness constexpr where applicable, this function is overloaded so that the variant to use has the signature:
        int length();
    */
    int length(typename std::conditional<has_fixed_length, detail::not_implemented, void>::type)
    {
        return members.length;
    }

    T* channel(int ch)
    {
        errorif(ch < 0 || ch >= fields.ptrs.size(), "channel out of range");
        return members.ptrs[ch];
    }
};

template <int NumChannels=-PURO_BUFFER_DEFAULT_MAX_CHANNELS, int Length=-1>
using bufferf = buffer<float, NumChannels, Length>;

template <int NumChannels=-PURO_BUFFER_DEFAULT_MAX_CHANNELS, int Length=-1>
using bufferd = buffer<double, NumChannels, Length>;



////////////////////////////////
// buffer operations
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
BufferType buffer_trim_to_length(BufferType buffer, int newLength) noexcept
{
    errorif(newLength > buffer.numSamples, "new length out of bounds");

    buffer.numSamples = math::max(newLength, 0);
    return buffer;
}

template <typename BufferType>
BufferType buffer_trim_from_end(BufferType buffer, int trimLength) noexcept
{
    errorif(newLength > buffer.numSamples, "new length out of bounds");

    buffer.numSamples -= trimLength;
    return buffer;
}

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

/** Create a Buffer with the data laid out into the provided vector.
    The vector may be resized if needed depending on template arg. Number of channels is deducted from the template args. */
template <typename BufferType, typename FloatType, bool resizeIfNeeded = PURO_BUFFER_WRAP_VECTOR_RESIZING>
BufferType buffer_wrap_vector(std::vector<FloatType>& vector, int numSamples) noexcept
{
    if (resizeIfNeeded)
    {
        const int totLength = BufferType::num_channels * numSamples;

        if ((int)vector.size() < totLength)
            vector.resize(totLength);
    }

    return BufferType(numSamples, vector.data());
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

template <typename BufferType, typename MultBufferType>
BufferType buffer_multiply(BufferType dst, const MultBufferType src) noexcept
{
    errorif(dst.length() != src.length(), "dst and src buffer lengths don't match");

    // identical channel config
    if (dst.getNumChannels() == src.getNumChannels())
    {
        for (int ch = 0; ch < dst.getNumChannels(); ++ch)
        {
            math::multiply_inplace(dst.channel(ch), src.channel(ch), dst.length());
        }
    }
    // mono src, multichannel dst
    else if (dst.getNumChannels() != src.getNumChannels() && src.getNumChannels() == 1)
    {
        for (int ch = 0; ch < dst.getNumChannels(); ++ch)
        {
            math::multiply_inplace(dst.channel(ch), src.channel(0), dst.length());
        }
    }
       
    return dst;
}

template <typename BufferType>
void buffer_clear(BufferType buf) noexcept
{
    for (int ch=0; ch<buffer.getNumChannels(); ++ch)
    {
        math::set<typename BufferType::value_type>(buf.channel(ch), 0, buf.length());
    }
}

} // namespace puro
