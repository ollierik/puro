#pragma once

template <class FloatType>
class ConstEnvelope
{
public:

    ConstEnvelope()
    {
    }

};


#if 0
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

    void getNextOutput(Buffer<FloatType>& envelope, int startIndex, int numSamples)
    {
        for (int i = startIndex; i < numSamples; ++i)
        {
            const FloatType y = position * 2 * Math::pi<FloatType>();
            envelope[i] = (1 - y) / 2;
            position += increment;
        }

        // calculate
        Math::cos(envelope.getPtr(0, startIndex), numSamples);

        // copy to other channels
        for (int ch = 1; ch < envelope.numChannels; ++ch)
        {
            FloatType* dst = envelope.getPtr(ch, startIndex);
            const FloatType* src = envelope.getPtr(0, startIndex);
            Math::multiplySet(dst, src, numSamples);
        }

    }

private:

    const FloatType increment;
    FloatType position;
};
#endif
