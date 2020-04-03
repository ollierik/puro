#pragma once

template <class FloatType>
class SineEnvelope
{
public:

    SineEnvelope(int lengthInSamples)
        : increment(static_cast<FloatType>(1.0) / static_cast<FloatType>(lengthInSamples))
        , position(0)
    {
    }

    void getNextOutput(FloatType* vec, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {

            const FloatType y = position * Math::pi<FloatType>();
            vec[i] = y;
            position += increment;
        }

        Math::sin(vec, numSamples);
    }

private:

    const FloatType increment;
    FloatType position;
};

template <class FloatType>
class HannEnvelope
{
public:

    HannEnvelope(int lengthInSamples)
        : increment(static_cast<FloatType>(1.0) / static_cast<FloatType>(lengthInSamples))
        , position(0)
    {
    }

    void getNextOutput(FloatType* vec, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {

            const FloatType y = position * 2 * Math::pi<FloatType>();
            vec[i] =  (1-y)/2;
            position += increment;
        }

        Math::cos(vec, numSamples);
    }

private:

    const FloatType increment;
    FloatType position;
};
