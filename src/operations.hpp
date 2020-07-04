#pragma once

namespace puro {

template <typename BufferType, typename MultBufferType>
BufferType content_multiply_inplace(BufferType dst, const MultBufferType src) noexcept
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
void constant_fill(BufferType buffer, typename BufferType::value_type value) noexcept
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        math::set(buffer.channel(ch), value, buffer.length());
    }
}


template <typename BufferType>
void noise_fill(BufferType buffer) noexcept
{
    using FloatType = typename BufferType::value_type;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto dst = buffer.channel(ch);
        for (int i = 0; i < buffer.numSamples; ++i)
        {
            const FloatType coef = static_cast<FloatType> (1) / static_cast<FloatType> (RAND_MAX / 2);
            const FloatType r = static_cast<FloatType> (std::rand()) * coef - 1;

            dst[i] = r;
        }
    }
}

template <typename BufferType, typename ValueType>
void linspace_fill(BufferType buffer, ValueType start, const ValueType increment) noexcept
{
    for (int i=0; i<buffer.length(); ++i)
    {
        buffer(0, i) = start;
        start += increment;
    }

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
    {
        math::copy(buffer.channel(ch), buffer.channel(0), buffer.length());
    }
}

/////////////////////////////////////////////
// Envelope sources
/////////////////////////////////////////////

template <typename FloatType>
Sequence<FloatType> content_envelope_halfcos_create_seq(int lengthInSamples) noexcept
{
    const FloatType val = math::pi<FloatType>() / static_cast<FloatType>(lengthInSamples + 1);
    return { val, val };
}

template <typename BufferType, typename SeqType>
SeqType content_envelope_halfcos_fill(BufferType buffer, SeqType seq) noexcept
{
    auto dst = buffer.channel(0);
    for (int i = 0; i < buffer.length(); ++i)
    {
        dst[i] = seq.value;
        seq.value += seq.increment;
    }

    math::sin(dst, buffer.length());

    // copy to other channels
    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
    {
        math::copy(buffer.channel(ch), buffer.channel(0), buffer.length());
    }

    return seq;
}

template <typename FloatType>
Sequence<FloatType> envl_hann_create_seq(int lengthInSamples, bool symmetric = true) noexcept
{
    const FloatType div = static_cast<FloatType>(lengthInSamples) + (symmetric ? 1 : 0);
    const FloatType val = 2 * math::pi<FloatType>() / div;

    return { val, val };
}

template <typename BufferType, typename SeqType>
SeqType envl_hann_fill(BufferType buffer, SeqType seq) noexcept
{
    for (int i = 0; i < buffer.length(); ++i)
    {
        const auto sample = (1 - cos(seq++)) / 2;
        buffer(0, i) = sample;
    }

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
    {
        math::copy(buffer.channel(ch), &buffer.channel(ch), buffer.length());
    }

    return seq;
}


template <typename BufferType, typename SrcBufferType, typename SeqType>
std::tuple <BufferType, SeqType> buffer_fill(BufferType buffer, SrcBufferType source, SeqType seq) noexcept
{
    using FloatType = typename BufferType::value_type;

    // source buffer will run out, trim the destination buffer
    if (source.length() < (seq + buffer.length()))
    {
        buffer = trimmed_length(buffer, source.length() - seq);
    }

    // identical channel config
    if (buffer.getNumChannels() == source.getNumChannels())
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            math::copy<FloatType>(buffer.channel(ch), source.channel(ch), buffer.length());
        }
        seq += buffer.length();
    }
    // mono source, use for all channels
    else if (source.getNumChannels() == 1)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            math::copy(buffer.channel(ch), source.channel(0), buffer.length());
        }
        seq += buffer.length();
    }
    else
    {
        errorif(true, "channel config combination not implemented");
    }

    return std::make_tuple(buffer, seq);
}


template <typename FloatType>
int content_interpolation_num_samples_available(int length, FloatType position, const FloatType rate, const int interp_order) noexcept
{
    return (int)std::ceil((static_cast<FloatType>(length-interp_order) - position)/rate);
}

template <typename BufferType, typename SeqType>
BufferType content_interp1_crop_buffer(BufferType buffer, int samplesAvailable, SeqType seq) noexcept
{
    const int interpOrder = 1;
    const int numAvailable = content_interpolation_num_samples_available(samplesAvailable, seq.value, seq.increment, interpOrder);

    if (numAvailable < buffer.length())
        buffer = puro::buffer_trim_length(buffer, numAvailable);

    return buffer;
}

template <typename BufferType, typename SeqType>
BufferType content_interp3_crop_buffer(BufferType buffer, int samplesAvailable, SeqType seq) noexcept
{
    const int interpOrder = 3;
    const int numAvailable = content_interpolation_num_samples_available(samplesAvailable, seq.value, seq.increment, interpOrder);

    if (numAvailable < buffer.length())
        buffer = puro::buffer_trim_length(buffer, numAvailable);

    return buffer;
}

/** Assumes that the source buffer can provide all the required samples, i.e. doesn't do bound checking.
    Buffer should be cropped for example with buffer_crop_for_interp before-hand. */
template <typename BufferType, typename SrcBufferType, typename SeqType>
std::tuple <BufferType, SeqType> content_interpolation1_fill(BufferType buffer, SrcBufferType source, SeqType seq) noexcept
{
    using FloatType = typename BufferType::value_type;

    errorif((buffer.getNumChannels() != source.getNumChannels())
            && (source.getNumChannels() != 1),
            "channel configuration not implemented");

    // identical channel config
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        //SeqType chSeq = seq;
        auto position = seq.value;
        const auto increment = seq.increment;

        auto dst = buffer.channel(ch);
        auto src = source.channel(ch);

        for (int i = 0; i < buffer.length(); ++i)
        {
            const int index = static_cast<int> (position);
            position += increment;
            const FloatType fract = static_cast<FloatType>(position - index);

            dst[i] = src[index] * (1 - fract) + src[index + 1] * fract;
        }

        if (ch == buffer.getNumChannels() - 1)
            seq.value = position;
    }

    return std::make_tuple(std::move(buffer), std::move(seq));
}

template <typename BufferType, typename SrcBufferType, typename SeqType>
std::tuple <BufferType, SeqType> content_interp3_fill(BufferType buffer, SrcBufferType source, SeqType seq) noexcept
{
    using FloatType = typename BufferType::value_type;

    errorif((buffer.getNumChannels() != source.getNumChannels())
            && (source.getNumChannels() != 1),
            "channel configuration not implemented");

    // identical channel config
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        //SeqType chSeq = seq;
        auto position = seq.value;
        const auto increment = seq.increment;

        auto dst = buffer.channel(ch);
        auto src = source.channel(ch);

        for (int i = 0; i < buffer.length(); ++i)
        {
            const int index = static_cast<int> (position);
            position += increment;
            const FloatType fract = static_cast<FloatType>(position - index);

            const auto a = src[index-1];
            const auto b = src[index];
            const auto c = src[index+1];
            const auto d = src[index+2];
            const auto cminusb = c-b;

            // pure data
            dst[i] = b + fract * (cminusb - 0.1666667f * (1.0f - fract) * ((d - a - 3.0f * cminusb) * fract + (d + 2.0f*a - 3.0f*b)));
        }

        if (ch == buffer.getNumChannels() - 1)
            seq.value = position;
    }

    return std::make_tuple(std::move(buffer), std::move(seq));

}


} // namespace puro
