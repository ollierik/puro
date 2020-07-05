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

template <int interpolationOrder=3, typename PositionType>
std::tuple<RelativeAlignment, PositionType> interp_avoid_out_of_bounds_reads(RelativeAlignment alignment, PositionType readPos, const PositionType readInc, const int sourceLength) noexcept
{
    int prepad;
    int postpad;
    if constexpr (interpolationOrder == 3)
    {
        prepad = 1;
        postpad = 2;
    }
    else if constexpr (interpolationOrder == 1)
    {
        prepad = 0;
        postpad = 1;
    }

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
template <typename BufferType, typename SourceBufferType, typename PositionType>
PositionType interp1_fill(BufferType buffer, SourceBufferType source, const PositionType readPos, const PositionType increment) noexcept
{
    using FloatType = typename BufferType::value_type;

    errorif((buffer.getNumChannels() != source.getNumChannels())
        && (source.getNumChannels() != 1),
        "channel configuration not implemented");

    auto position = readPos;

    // identical channel config
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        //SeqType chSeq = seq;
        auto position = readPos;

        auto dst = buffer.channel(ch);
        auto src = source.channel(ch);

        for (int i = 0; i < buffer.length(); ++i)
        {
            const int index = static_cast<int> (position);
            position += increment;
            const FloatType fract = static_cast<FloatType>(position - index);

            dst[i] = src[index] * (1 - fract) + src[index + 1] * fract;
        }
    }

    return position;
}

/** Assumes that the source buffer can provide all the required samples, i.e. doesn't do bound checking.
    Buffer should be cropped for example with interp_crop_buffer before-hand. */
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

            const FloatType* x = &src[index-1];
            dst[i] = x[1] + fract * (x[2]-x[1]- static_cast<FloatType>(0.1666667) * (1-fract)
                    * ( (x[3]-x[0] - 3.0f*(x[2]-x[1]))*fract
                      + (x[3] + 2*x[0] - 3*x[1])));
        }
    }

    return position;
}

} // namespace puro
