#pragma once

namespace puro {

template <typename FloatType, int numChannels>
struct PanCoeffs
{
    constexpr int getNumChannels() { return numChannels; };
    std::array<FloatType, numChannels*numChannels> coeffs;

    FloatType operator() (int fromCh, int toCh)
    {
        return coeffs[numChannels * fromCh + toCh];
    }
};

/** Pan range [-1, 1], where -1 is hard left and 1 is hard right. */
template <typename FloatType>
PanCoeffs<FloatType, 2> pan_create_stereo(FloatType pan)
{
    // [ 0->0, 0->1, 1->0, 1->1 ]
    if (pan <= 0)
    {
        const FloatType ltol = 1;
        const FloatType ltor = 0;
        const FloatType rtol = -pan;
        const FloatType rtor = (1 + pan);
        return PanCoeffs<FloatType, 2> { ltol, ltor, rtol, rtor };
    }
    else
    {
        const FloatType ltol = 1 - pan;
        const FloatType ltor = pan;
        const FloatType rtol = 0;
        const FloatType rtor = 1;
        return PanCoeffs<FloatType, 2> { ltol, ltor, rtol, rtor };
    }
}

template <typename BufferType, typename PanType>
void pan_apply(BufferType dst, BufferType src, PanType coeffs)
{
    errorif(src.getNumChannels() != coeffs.getNumChannels(), "channel configs between src and coeffs don't match");
    errorif(dst.getNumChannels() != coeffs.getNumChannels(), "channel configs between dst and coeffs don't match");

    const auto numChannels = coeffs.getNumChannels();
    using FloatType = typename BufferType::value_type;

    puro::buffer_clear(dst);

    // TODO optimise for special cases coef == 0 and coef == 1

    for (int fromCh = 0; fromCh < numChannels; ++fromCh)
    {
        for (int toCh = 0; toCh < numChannels; ++toCh)
        {
            const auto coef = coeffs(fromCh, toCh);
            math::multiply_add(dst.channel(toCh), src.channel(fromCh), coef, dst.length());
        }
    }
}

} // namespace puro
