#pragma once

namespace puro {

template <typename FloatType>
int interp_num_samples_available(int length, FloatType position, const FloatType rate, const int interpOrder) noexcept
{
    return (int)std::ceil((static_cast<FloatType>(length - interpOrder) - position) / rate);
}

template <typename BufferType, typename SeqType>
BufferType interp1_crop_buffer(BufferType buffer, int samplesAvailable, SeqType seq) noexcept
{
    const int interpOrder = 1;
    const int numAvailable = interp_num_samples_available(samplesAvailable, seq.value, seq.increment, interpOrder);

    if (numAvailable < buffer.length())
        buffer = puro::buffer_trim_length(buffer, numAvailable);

    return buffer;
}

template <typename PositionType>
std::tuple<RelativeAlignment, PositionType> interp3_avoid_out_of_bounds_reads(RelativeAlignment alignment, PositionType readPos, const PositionType readInc, const int sourceLength) noexcept
{
    const int prepad = 1;
    const int postpad = 2;

    int startReadIndex = static_cast<int> (readPos);
    int sourceLengthNeeded = static_cast<int> (readPos + alignment.remaining * readInc) + postpad;

    if (sourceLength < sourceLengthNeeded)
    {
        const int delta = sourceLengthNeeded - sourceLength;
        alignment.remaining -= delta;
    }
    if (startReadIndex < prepad)
    {
        readPos = prepad;
        alignment.remaining -= prepad;
        alignment.offset += prepad;
    }

    return std::make_tuple(std::move(alignment), std::move(readPos));
}

/** Assumes that the source buffer can provide all the required samples, i.e. doesn't do bound checking.
    Buffer should be cropped for example with interp_crop_buffer before-hand. */
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

template <typename BufferType, typename SourceBufferType, typename PositionType>
PositionType interp3_fill(BufferType buffer, SourceBufferType source, const PositionType readPos, const PositionType increment) noexcept
{
    using FloatType = typename BufferType::value_type;

    errorif((buffer.getNumChannels() != source.getNumChannels()) && (source.getNumChannels() != 1),
        "channel configuration not implemented");

    PositionType position = readPos;

    // identical channel config
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        //SeqType chSeq = seq;
        position = readPos;

        auto dst = buffer.channel(ch);
        auto src = source.channel(ch);

        for (int i = 0; i < buffer.length(); ++i)
        {
            const int index = static_cast<int> (position);
            position += increment;
            const FloatType fract = static_cast<FloatType>(position - index);

            const auto a = src[index - 1];
            const auto b = src[index];
            const auto c = src[index + 1];
            const auto d = src[index + 2];
            const auto cminusb = c - b;

            // pure data
            dst[i] = b + fract * (cminusb - 0.1666667 * (1.0 - fract) * ((d - a - 3.0 * cminusb) * fract + (d + 2.0*a - 3.0*b)));
        }
    }

    return position;
}

} // namespace puro
