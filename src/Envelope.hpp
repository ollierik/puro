#pragma once

#include <cmath>

template <class FloatType>
class EnvelopeTemplate
{
public:

    EnvelopeTemplate(int lengthInSamples)
        : increment(1.0 / static_cast<double>(lengthInSamples))
        , position(0)
    {
    }

    FloatType getNext()
    {
        // TODO template sin and sinf
        const double pi = 3.14159265359;
        const FloatType y = std::sinf(static_cast<FloatType> (position * pi));
        position += increment;
        return y;
    }

private:

    const double increment;
    double position;
};
