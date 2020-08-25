#pragma once

namespace puro {

template <typename ValueType>
struct Timer
{
    Timer(ValueType interval) noexcept : interval(interval), counter(0) {}

    ValueType advance(ValueType units) noexcept
    {
        counter += units;

        if (counter <= interval)
            return 0;

        units = counter - interval;
        counter = 0;

        return units;
    }

    ValueType interval;
    ValueType counter;
};

template <typename FloatType>
FloatType random_bspline2_fast(std::mt19937& gen, FloatType mean, FloatType dev) noexcept
{
    constexpr int smin = std::numeric_limits<short>::min();
    constexpr int smax = std::numeric_limits<short>::max();
    constexpr FloatType fdiv = static_cast<FloatType>(1.0 / (3.0 * static_cast<FloatType>(smax)));

    std::uniform_int_distribution<short> dist (smin, smax);

    const FloatType r = fdiv * (static_cast<float>(dist(gen)) + static_cast<float>(dist(gen)) + static_cast<float>(dist(gen)));

    return r * dev + mean;
}


template <typename ValueType, bool useRelativeDeviation, typename FloatType=float>
struct Parameter
{
public:
    Parameter(ValueType min, ValueType max) noexcept
        : generator(std::random_device()()), centre(0), deviation(0), minimum(min), maximum(max)
    {}

    Parameter(FloatType centre, FloatType deviation, ValueType min, ValueType max) noexcept
        : generator(std::random_device()()), centre(centre), deviation(deviation), minimum(min), maximum(max)
    {}

    ValueType get() noexcept
    {
        FloatType f;
        if (useRelativeDeviation)
            f = random_bspline2_fast(generator, centre, centre * deviation);
        else
            f = random_bspline2_fast(generator, centre, deviation);

        if (std::is_integral<ValueType>::value)
        {
            const ValueType i = static_cast<ValueType> (math::round<FloatType, ValueType>(f));
            return math::clip<ValueType> (i, minimum, maximum);
        }
        return math::clip<ValueType> (static_cast<ValueType>(f), minimum, maximum);
    }

    std::mt19937 generator;

    FloatType centre;
    FloatType deviation;
    ValueType minimum;
    ValueType maximum;
};
    
    
template <typename BufferType, typename JuceBufferType>
BufferType buffer_from_juce_buffer(JuceBufferType& juceBuffer)
{
    BufferType buffer (juceBuffer.getNumSamples());

    if (juceBuffer.getNumChannels() == 1)
    {
        for (int ch=0; ch < buffer.num_channels(); ++ch)
            buffer.ptrs[ch] = juceBuffer.getWritePointer(0);
    }
    else if (juceBuffer.getNumChannels() == buffer.num_channels())
    {
        for (int ch=0; ch < buffer.num_channels(); ++ch)
            buffer.ptrs[ch] = juceBuffer.getWritePointer(ch);
    }
    else
    {
        errorif(true, "channel config not implemented");
    }

    return buffer;
}
    
}
