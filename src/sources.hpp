#pragma once

namespace puro {

template <typename BufferType>
BufferType noise_fill(BufferType buffer)
{
    using FloatType = typename BufferType::value_type;

    for (int ch=0; ch < buffer.getNumChannels(); ++ch)
    {
        for (int i=0; i < buffer.numSamples; ++i)
        {
            const FloatType coef = static_cast<FloatType> (1) / static_cast<FloatType> (RAND_MAX/2);
            const FloatType r = static_cast<FloatType> (std::rand()) * coef - 1;

            buffer(ch, i) = r;
        }
    }
    return buffer;
}

template <typename BufferType, typename ValueType>
BufferType constant_fill(BufferType buffer, const ValueType value)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        math::set(&buffer(ch, 0), buffer.size(), value);
    }

    return buffer;
}

/////////////////////////////////////////////
// Envelope sources
/////////////////////////////////////////////

template <typename FloatType>
struct RatioRange
{
    FloatType position;
    const FloatType increment;
};

template <typename FloatType>
RatioRange<FloatType> envelope_halfcos_create_range(int lengthInSamples)
{
    const FloatType val = math::pi<FloatType>() / static_cast<FloatType>(lengthInSamples-1);
    return {val, val};
}

template <typename BufferType, typename FloatType>
BufferType envelope_halfcos_fill(BufferType buffer, RatioRange<FloatType> range)
{
    for (int i=0; i<buffer.size(); ++i)
    {
        buffer(0, i) = range.position;
        range.position += range.increment;
    }

    math::sin(&buffer(0, 0), buffer.size());

    // copy to other channels
    for (int ch=1; ch < buffer.getNumChannels(); ++ch)
    {
        math::copy(&buffer(ch, 0), &buffer(0, 0), buffer.size());
    }

    return buffer;
}


template <typename FloatType>
RatioRange<FloatType> envelope_hann_create_range(int lengthInSamples, bool symmetric=true)
{
    const FloatType div = static_cast<FloatType>(lengthInSamples) + (symmetric ? 1 : 0);
    const FloatType val = 2*math::pi<FloatType>() / div;

    return {val, val};
}

template <typename BufferType, typename FloatType>
BufferType envelope_hann_fill(BufferType buffer, RatioRange<FloatType> range)
{
    for (int i = 0; i < buffer.size(); ++i)
    {
        const FloatType sample = (1 - cos(range.position)) / 2;
        buffer(0, i) = sample;
        range.position += range.increment;
    }

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
    {
        FloatType* dst = buffer.channel(ch);
        math::copy(dst, &buffer(ch, 0), buffer.size());
    }

    return buffer;
}


/*
template <typename SourceBufferType>
class BufferSource
{
public:

    BufferSource(SourceBufferType& buffer, int startIndex)
        : sourceBuffer(buffer)
        , index(startIndex)
    {
    }

    template <typename BufferType>
    BufferType next(bops::Type opType, BufferType buffer)
    {
        using FloatType = typename BufferType::value_type;

        // buffer will run out, trim it
        if (sourceBuffer.size() < index + buffer.size())
        {
            buffer = bops::trimmed_length(buffer, sourceBuffer.size() - index);
        }

        // identical channel config
        if (buffer.getNumChannels() == sourceBuffer.getNumChannels())
        {
            for (int ch=0; ch < buffer.getNumChannels(); ++ch)
            {
                FloatType* dst = buffer.channel(ch);
                FloatType* src = &sourceBuffer(ch, index);

                if (opType == bops::Type::add)
                    math::add<FloatType>(dst, src, buffer.size());
                else
                    math::copy<FloatType>(dst, src, buffer.size());
            }
            index += buffer.size();
        }
        // mono source, use for all channels
        else if (sourceBuffer.getNumChannels() == 1)
        {
            FloatType* src = &sourceBuffer(0, index);

            for (int ch=0; ch<buffer.getNumChannels(); ++ch)
            {
                FloatType* dst = buffer.channel(ch);

                if (opType == bops::Type::add)
                    math::add(dst, src, buffer.size());
                else
                    math::copy(dst, src, buffer.size());
            }
            index += buffer.size();
        }
        else
        {
            errorif(true, "channel config combination not implemented");
        }

        return buffer;
    }

    SourceBufferType& sourceBuffer;
    int index;
};



template <typename BufferType>
BufferType multiply_add(BufferType dst, const BufferType src1, const BufferType src2)
{
    errorif(!(dst.size() == src1.size()), "dst and src1 buffer lengths don't match");
    errorif(!(dst.size() == src2.size()), "dst and src2 buffer lengths don't match");

    for (int ch = 0; ch < dst.getNumChannels(); ++ch)
    {
        math::multiply_add(dst.channel(ch), src1.channel(ch), src2.channel(ch), dst.size());
    }

    return dst;
}

*/

};
