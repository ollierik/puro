#pragma once

namespace puro {


template <typename PositionType>
PositionType envelope_halfcos_get_increment(int lengthInSamples) noexcept
{
    const PositionType val = math::pi / static_cast<PositionType>(lengthInSamples + 1);
    return val;
}

template <typename BufferType, typename PositionType>
PositionType envelope_halfcos_fill(BufferType buffer, PositionType position, const PositionType increment) noexcept
{
    auto dst = buffer.channel(0);
    for (int i = 0; i < buffer.length(); ++i)
    {
        dst[i] = position;
        position += increment;
    }

    math::sin(dst, buffer.length());

    // copy to other channels
    for (int ch = 1; ch < buffer.num_channels(); ++ch)
    {
        math::copy(buffer.channel(ch), buffer.channel(0), buffer.length());
    }

    return position;
}

template <typename PositionType>
PositionType envelope_hann_get_increment(int lengthInSamples, bool symmetric = true) noexcept
{
    const PositionType div = static_cast<PositionType>(lengthInSamples) + (symmetric ? 1 : 0);
    const PositionType val = 2 * math::pi / div;
    return val;
}

template <typename BufferType, typename PositionType>
PositionType envelope_hann_fill(BufferType buffer, PositionType position, const PositionType increment) noexcept
{
    auto dst = buffer.channel(0);
    for (int i = 0; i < buffer.length(); ++i)
    {
        const auto sample = (1 - std::cos(position)) / 2;
        dst[i] = sample;
        position += increment;
    }

    for (int ch = 1; ch < buffer.num_channels(); ++ch)
    {
        math::copy(buffer.channel(ch), buffer.channel(0), buffer.length());
    }

    return position;
}

} // namespace puro

