#pragma once

namespace puro {


template <typename T> struct is_scalar {}; 
template <> struct is_scalar <int>                  { typedef void type; };
template <> struct is_scalar <unsigned int>         { typedef void type; };
template <> struct is_scalar <float>                { typedef void type; };
template <> struct is_scalar <double>               { typedef void type; };
template <> struct is_scalar <const int>            { typedef void type; };
template <> struct is_scalar <const unsigned int>   { typedef void type; };
template <> struct is_scalar <const float>          { typedef void type; };
template <> struct is_scalar <const double>         { typedef void type; };


/*
template <typename BufferType>
BufferType buffer_trim_begin(BufferType buffer, int offset) noexcept
{
    errorif(offset < 0 || offset > buffer.numSamples, "offset out of bounds");

    buffer.numSamples -= offset;

    for (int ch = 0; ch < buffer.num_channels(); ++ch)
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
*/

/// Get a sub segment of a buffer with given offset and length
template <typename BT>
BT segment(BT buffer, int offset, int length) noexcept
{
    errorif(offset > buffer.numSamples, "segment offset greater than number of samples available");
    errorif(length < 0 || length > (offset + buffer.length()), "segment length out of bounds");

    for (int ch = 0; ch < buffer.num_channels(); ++ch)
        buffer.channelPtrs[ch] = &buffer.channelPtrs[ch][offset];

        buffer.numSamples = length;
        return buffer;
}

/// Get a segment of a buffer from start to end index
template <typename BT>
BT slice(BT buffer, int start, int end) noexcept
{
    errorif(start < 0, "slice start below zero");
    errorif(start > buffer.length(), "slice start greater than number of samples available");
    errorif(end < start, "slice end below start");
    errorif(end > buffer.length(), "slice end greater than number of samples available");

    for (int ch = 0; ch < buffer.num_channels(); ++ch)
        buffer.ptrs[ch] = &buffer.ptrs[ch][start];

        buffer.num_samples = end - start;
        return buffer;
}

/// Split the given buffer into from index. The second buffer starts with index at zeroeth index.
template <typename BT>
std::tuple<BT, BT> split(BT buffer, int index) noexcept
{
    errorif(index <= 0, "split is 0 or below");
    errorif(index >= buffer.numSamples, "split greater than number of samples available");

    BT pre = buffer_trim_length(buffer, index);
    BT post = buffer_trim_begin(buffer, index);

    return std::make_tuple(std::move(pre), std::move(post));
}


/** Create a Buffer with the data laid out into the provided vector.
 The vector may be resized if needed depending on template arg. Number of channels is deducted from the template args. */
/*
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
*/

/*
template <typename ToBufferType, typename FromBufferType>
ToBufferType buffer_convert_to_type(FromBufferType src) noexcept
{
    ToBufferType dst (src.length());
    for (int ch=0; ch < dst.num_channels(); ++ch)
    {
        errorif (ch >= src.num_channels(), "trying to convert from less channels to a larger one");
        dst.channelPtrs[ch] = src.channelPtrs[ch];
    }
    return dst;
}
*/

/*
template <typename BufferType, typename FloatType>
BufferType fit_vector_into_dynamic_buffer(std::vector<FloatType>& vector, int numChannels, int numSamples) noexcept
{
    const int totLength = numChannels * numSamples;

    // resize if needed
    if ((int)vector.size() < totLength)
        vector.resize(totLength);

    return BufferType(numChannels, numSamples, vector.data());
}
 */

template <typename BT1, typename BT2, typename BT3>
void multiply_add(BT1 dst, const BT2 src1, const BT3 src2) noexcept
{
    errorif(!(dst.length() == src1.length()), "dst and src1 buffer lengths don't match");
    errorif(!(dst.length() == src2.length()), "dst and src2 buffer lengths don't match");

    // identical channel configs
    if (src1.num_channels() == src2.num_channels())
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            math::multiply_add(dst.channel(ch), src1.channel(ch), src2.channel(ch), dst.length());
        }
    }
    // src2 is a mono buffer
    else if (src1.num_channels() > 1 && src2.num_channels() == 1)
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            //math::multiply_add(dst.channel(ch), src1.channel(ch), src2.channel(0), dst.length());
            math::multiply_add(dst.channel(ch), src1.channel(ch), src2.channel(0), dst.length());
        }
    }
    else
    {
        errorif(true, "channel config not implemented");
    }
}

template <typename BT1, typename BT2, typename ValueType,
          typename Enable = is_scalar<ValueType>>
void multiply_add(BT1 dst, const BT2 src, const ValueType multiplier) noexcept
{
    errorif(dst.num_channels() != src.num_channels(), "dst and src channel number doesn't match");
    errorif(dst.length() != src.length(), "dst and src1 buffer lengths don't match");

    // identical channel configs
    for (int ch = 0; ch < dst.num_channels(); ++ch)
    {
        math::multiply_add(dst.channel(ch), src.channel(ch), multiplier, dst.length());
    }
}

template <typename BT>
void multiply(BT dst, const typename BT::value_type value) noexcept
{
    for (int ch = 0; ch < dst.num_channels(); ++ch)
    {
        math::multiply(dst[ch], value, dst.length());
    }
}

template <typename BT1, typename BT2>
void multiply(BT1 dst, const BT2 src) noexcept
{
    errorif(dst.length() != src.length(), "dst and src buffer lengths don't match");

    // identical channel config
    if (dst.num_channels() == src.num_channels())
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            math::multiply(dst.channel(ch), src.channel(ch), dst.length());
        }
    }
    // mono src, multichannel dst
    else if (dst.num_channels() != src.num_channels() && src.num_channels() == 1)
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            math::multiply(dst.channel(ch), src.channel(0), dst.length());
        }
    }

    return dst;
}

template <typename BT1, typename BT2>
void multiply(BT1 dst, BT2 src, typename BT1::value_type value) noexcept
{
    errorif(dst.length() != src.length(), "dst and src buffer lengths don't match");

    // identical channel config
    for (int ch = 0; ch < dst.num_channels(); ++ch)
    {
        math::multiply(dst.channel(ch), src.channel(ch), value, dst.length());
    }
}

template <typename BT1, typename BT2>
void add(BT1 dst, const BT2 src) noexcept
{
    errorif(dst.length() != src.length(), "dst and src buffer lengths don't match");

    // identical channel config
    if (dst.num_channels() == src.num_channels())
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            math::add(dst[ch], src[ch], dst.length());
        }
    }
    // mono src, multichannel dst
    else if (dst.num_channels() != src.num_channels() && src.num_channels() == 1)
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            math::add(dst[ch], src[0], dst.length());
        }
    }

    return dst;
}

template <typename BT>
void add(BT dst, const typename BT::value_type value) noexcept
{
    for (int ch = 0; ch < dst.num_channels(); ++ch)
    {
        math::add(dst[ch], value, dst.length());
    }

    return dst;
}

template <typename BT1, typename BT2>
void buffer_substract(BT1 dst, const BT2 src) noexcept
{
    errorif(dst.length() != src.length(), "dst and src buffer lengths don't match");

    // identical channel config
    if (dst.num_channels() == src.num_channels())
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            math::substract(dst.channel(ch), src.channel(ch), dst.length());
        }
    }
    // mono src, multichannel dst
    else if (dst.num_channels() != src.num_channels() && src.num_channels() == 1)
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            math::substract(dst.channel(ch), src.channel(0), dst.length());
        }
    }

    return dst;
}

template <typename BT1, typename BT2>
void copy(BT1 dst, BT2 src) noexcept
{
    errorif(dst.num_channels() != src.num_channels(), "dst and src channel number doesn't match");
    errorif(dst.length() != src.length(), "dst and src lengths don't match");

    for (int ch=0; ch<dst.num_channels(); ++ch)
    {
        math::copy(dst[ch], src[ch], dst.length());
    }
}

template <typename BT1, typename BT2>
void copy_decimating(BT1 dst, BT2 src, int ratio) noexcept
{
    errorif(dst.length() != src.length() / ratio, "dst.length (" << dst.length() << ") should be src.length/ratio (" << src.length() << "/" << ratio << ")");
    errorif(src.length() % ratio != 0, "src.length should be divisible by ratio");

    for (int ch=0; ch<dst.num_channels(); ++ch)
    {
        math::copy_decimating(dst[ch], src[ch], ratio, src.length());
    }
}

template <typename BT>
void clear(BT buffer) noexcept
{
    for (int ch=0; ch<buffer.num_channels(); ++ch)
    {
        math::set<typename BT::value_type>(buffer[ch], 0, buffer.length());
    }
}

template <typename BT>
void normalise(BT buffer)
{
    using T = typename BT::value_type;

    T max = 0;
    for (auto ch = 0; ch < buffer.num_channels(); ++ch)
    {
        T* ptr = buffer[ch];

        for (int i = 0; i < buffer.length(); ++i)
        {
            max = puro::math::max(abs(ptr[i]), max);
        }
    }

    if (max > 0)
    {
        for (auto ch = 0; ch < buffer.num_channels(); ++ch)
        {
            T* ptr = buffer[ch];
            for (auto ch=0; ch<buffer.num_channels(); ++ch)
            {
                math::multiply(ptr, static_cast<T>(1)/max, buffer.length());
            }
        }
    }
}

template <typename BT>
void max(BT buffer, const typename BT::value_type value)
{
    for (int ch=0; ch<buffer.num_channels(); ++ch)
    {
        math::max(buffer.channel(ch), value, buffer.length());
    }
}

template <typename BT>
void pow(BT buffer, const typename BT::value_type power)
{
    for (int ch=0; ch<buffer.num_channels(); ++ch)
    {
        math::pow(buffer.channel(ch), power, buffer.length());
    }
}

template <typename BT>
void reciprocal(BT buffer)
{
    for (int ch=0; ch<buffer.num_channels(); ++ch)
    {
        math::reciprocal(buffer.channel(ch), buffer.length());
    }
}

template <typename BT>
void log(BT buffer) noexcept
{
    for (int ch=0; ch<buffer.num_channels(); ++ch)
    {
        math::log(buffer[ch], buffer.length());
    }
}

template <typename BT>
void negate(BT buffer) noexcept
{
    for (int ch=0; ch<buffer.num_channels(); ++ch)
    {
        math::multiply(buffer[ch], static_cast<typename BT::value_type> (-1), buffer.length());
    }
}

template <typename BT>
typename BT::value_type sum(BT buffer) noexcept
{
    for (int ch=0; ch<buffer.num_channels(); ++ch)
    {
        math::sum(buffer[ch], buffer.length());
    }
}
    
} // namespace puro
