#pragma once

/** Scalar math functions */
namespace puro {
namespace math {

constexpr auto pi = 3.14159265358979323846;

template <typename ValueType>
ValueType min(ValueType f1, ValueType f2) noexcept
{
    return f1 < f2 ? f1 : f2;
}

template <typename ValueType>
ValueType max(ValueType f1, ValueType f2) noexcept
{
    return f1 > f2 ? f1 : f2;
}

template <typename ValueType>
ValueType clip(ValueType val, ValueType minValue, ValueType maxValue) noexcept
{
    return min(max(val, minValue), maxValue);
}

template <typename ValueType>
ValueType wrap(ValueType index, ValueType length) noexcept
{
    return (index + length) % length;
}

template <typename ValueType>
ValueType normalise(ValueType value, ValueType zero, ValueType one) noexcept
{
    return (value - zero) / (one - zero);
}

template <typename ValueType>
ValueType scale(ValueType value, ValueType min, ValueType max) noexcept
{
    return value * (max - min) + min;
}

template <typename ValueType>
ValueType atodb(ValueType value, ValueType nonzero=1e-30) noexcept
{
    return 20 * log10(value);
}

template <typename ValueType>
ValueType dbtoa(ValueType value) noexcept
{
    return pow(static_cast<ValueType>(10), value / static_cast<ValueType>(20));
}

/** Frequency to log2 scale */
template <typename ValueType>
ValueType ftox_log2(ValueType freq, const ValueType minLog2= -10.784634845557521, const ValueType maxLog2=-1)
{
    return normalise(log2(freq), minLog2, maxLog2);
}

/** Log2 scale normalised x to frequency */
template <typename ValueType>
ValueType xtof_log2(ValueType value, const ValueType minLog2= -10.784634845557521, const ValueType maxLog2=-1)
{
    return pow(static_cast<ValueType> (2), scale(value, minLog2, maxLog2));
}

template <typename FloatType, typename ValueType=int>
ValueType round(FloatType value) noexcept
{
    return static_cast<ValueType> (value + (FloatType)0.5);
}

/** Compare equality with error */
template <typename FloatType>
bool equal(FloatType f1, FloatType f2, const FloatType epsilon=1e-5)
{
    return (abs(f1-f2) < epsilon);
}

} // namespace math
} // namespace puro
