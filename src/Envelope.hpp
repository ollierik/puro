#pragma once

template <class FloatType>
class EnvelopeTemplate
{
public:

    EnvelopeTemplate(int lengthInSamples)
        : increment(1.0 / static_cast<double>(lengthInSamples))
        , position(0)
    {
    }

    void getNextOutput(FloatType* vec, int numSamples)
    {
        for (int i=0; i<numSamples; i++)
        {
            // TODO template sin and sinf
            const double pi = 3.14159265359;
            const FloatType y = std::sinf(static_cast<FloatType> (position * pi));
            position += increment;
            vec[i] = y;
        }
    }

private:

    const double increment;
    double position;
};
