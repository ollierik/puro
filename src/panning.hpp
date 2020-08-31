#pragma once

namespace puro {


/** Represents the coefficients used for panning.
    Multiplication coefficients are stored in an array, source channel first.
    So for stereo channel:
        [ leftToLeft, leftToRight, rightToLeft, rightToRight ]
*/
template <typename FloatType, int numChannels>
struct PanCoeffs
{
    constexpr int getNumChannels() noexcept { return numChannels; };
    std::array<FloatType, numChannels*numChannels> coeffs;

    FloatType operator() (int fromCh, int toCh) noexcept
    {
        return coeffs[numChannels * fromCh + toCh];
    }
};

/** Pan range is [-1, 1], where -1 is hard left and 1 is hard right. */
    
template <typename FloatType>
PanCoeffs<FloatType, 2> pan_create_stereo(FloatType pan) noexcept
{
    // [ leftToLeft, leftToRight, rightToLeft, rightToRight ]
    if (pan <= 0)
    {
        const FloatType ltol = 1;
        const FloatType ltor = 0;
        const FloatType rtol = -pan;
        const FloatType rtor = 1 + pan;
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
void content_pan_apply(BufferType dst, BufferType src, PanType coeffs) noexcept
{
    errorif(src.getNumChannels() != coeffs.getNumChannels(), "channel configs between src and coeffs don't match");
    errorif(dst.getNumChannels() != coeffs.getNumChannels(), "channel configs between dst and coeffs don't match");

    const auto numChannels = coeffs.getNumChannels();
    using FloatType = typename BufferType::value_type;

    dst.clear();

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

template <typename BufferType, typename SourceBufferType, typename PanType>
void content_pan_apply_and_add(BufferType dst, SourceBufferType src, PanType coeffs) noexcept
{
    using FloatType = typename BufferType::value_type;

    const int numChannels = coeffs.getNumChannels();

    if (dst.getNumChannels() == src.getNumChannels())
    {
        for (int fromCh = 0; fromCh < numChannels; ++fromCh)
        {
            for (int toCh = 0; toCh < numChannels; ++toCh)
            {
                const auto coef = coeffs(fromCh, toCh);
                math::multiply_add(dst.channel(toCh), src.channel(fromCh), coef, dst.length());
            }
        }
    }
    // mono source to multichannel
    else if (dst.getNumChannels() != src.getNumChannels() && src.getNumChannels() == 1)
    {
        for (int toCh = 0; toCh < numChannels; ++toCh)
        {
            FloatType coef = 0;
            for (int fromCh = 0; fromCh < coeffs.getNumChannels(); ++fromCh)
            {
                coef += coeffs(fromCh, toCh);
            }
            coef /= numChannels;

            math::multiply_add(dst.channel(toCh), src.channel(0), coef, dst.length());
        }
    }
}

} // namespace puro
