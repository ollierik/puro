#pragma once

namespace puro {


/*
template <typename T, T increment>
class ConstSequence
{
public:
    typedef T value_type;
    static constexpr int increment = increment;

    Sequence(T val) : value(val) {}

    T value;
};
*/

template <int increment = 1>
struct IndexSequence
{
    IndexSequence(int val) : value(val) {}

    IndexSequence operator++() // prefix
    {
        value += increment;
        return value;
    };

    IndexSequence operator++(int) // postfix
    {
        IndexSequence temp (value, increment);
        value += increment;
        return temp;
    };

    IndexSequence& operator+=(const int rhs)
    {
      value += rhs;
      return *this;
    }

    operator int() { return value; }

    int value;
};


template <typename T>
struct Sequence
{
    Sequence(T val, T inc) : value(val), increment(inc) {}

    Sequence operator++() // prefix
    {
        value += increment;
        return value;
    };

    Sequence operator++(int) // postfix
    {
        Sequence temp (value, increment);
        value += increment;
        return temp;
    };

    operator T() { return value; }

    T value;
    T increment;
};


///======================================================================================


template <typename BufferType>
BufferType noise_fill(BufferType buffer)
{
    using FloatType = typename BufferType::value_type;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        for (int i = 0; i < buffer.numSamples; ++i)
        {
            const FloatType coef = static_cast<FloatType> (1) / static_cast<FloatType> (RAND_MAX / 2);
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
        math::set(buffer.channel(ch), buffer.size(), value);
    }

    return buffer;
}

template <typename BufferType, typename ValueType>
BufferType linspace_fill(BufferType buffer, ValueType start, const ValueType increment)
{
    for (int i=0; i<buffer.size(); ++i)
    {
        buffer(0, i) = start;
        start += increment;
    }

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
    {
        math::copy(buffer.channel(ch), buffer.channel(0), buffer.size());
    }

    return buffer;
}

/////////////////////////////////////////////
// Envelope sources
/////////////////////////////////////////////

template <typename FloatType>
Sequence<FloatType> envl_halfcos_create_seq(int lengthInSamples)
{
    const FloatType val = math::pi<FloatType>() / static_cast<FloatType>(lengthInSamples + 1);
    return { val, val };
}

template <typename BufferType, typename SeqType>
std::tuple<BufferType, Sequence<float>> envl_halfcos_fill(BufferType buffer, SeqType seq)
{
    for (int i = 0; i < buffer.size(); ++i)
    {
        buffer(0, i) = seq++;
    }

    math::sin(&buffer(0, 0), buffer.size());

    // copy to other channels
    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
    {
        math::copy(buffer.channel(ch), buffer.channel(0), buffer.size());
    }

    return std::make_tuple<BufferType, Sequence<float>> (std::move(buffer), std::move(seq));
}

template <typename FloatType>
Sequence<FloatType> envl_hann_create_seq(int lengthInSamples, bool symmetric = true)
{
    const FloatType div = static_cast<FloatType>(lengthInSamples) + (symmetric ? 1 : 0);
    const FloatType val = 2 * math::pi<FloatType>() / div;

    return { val, val };
}

template <typename BufferType, typename SeqType>
std::tuple<BufferType, SeqType> envl_hann_fill(BufferType buffer, SeqType seq)
{
    for (int i = 0; i < buffer.size(); ++i)
    {
        const auto sample = (1 - cos(seq++)) / 2;
        buffer(0, i) = sample;
    }

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
    {
        math::copy(buffer.channel(ch), &buffer.channel(ch), buffer.size());
    }

    return std::make_tuple(std::move(buffer), std::move(seq));
}


template <typename BufferType, typename SrcBufferType, typename SeqType>
std::tuple <BufferType, SeqType> buffer_fill(BufferType buffer, SrcBufferType source, SeqType seq)
{
    using FloatType = typename BufferType::value_type;

    // source buffer will run out, trim the destination buffer
    if (source.size() < (seq + buffer.size()))
    {
        buffer = trimmed_length(buffer, source.size() - seq);
    }

    // identical channel config
    if (buffer.getNumChannels() == source.getNumChannels())
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            math::copy<FloatType>(buffer.channel(ch), source.channel(ch), buffer.size());
        }
        seq += buffer.size();
    }
    // mono source, use for all channels
    else if (source.getNumChannels() == 1)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            math::copy(buffer.channel(ch), source.channel(0), buffer.size());
        }
        seq += buffer.size();
    }
    else
    {
        errorif(true, "channel config combination not implemented");
    }

    return std::make_tuple(buffer, seq);
}


int num_samples_available_for_interp(int length, float position, const float rate, const int interp_order)
{

}

template <typename BufferType, typename SrcBufferType, typename SeqType>
std::tuple <BufferType, SeqType> buffer_interp1_fill(BufferType buffer, SrcBufferType source, SeqType seq)
{
    using FloatType = typename BufferType::value_type;

    // identical channel config
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        SeqType chSeq = seq;
        for (int i = 0; i < buffer.size(); ++i)
        {
            const FloatType pos = chSeq++;
            const int index = static_cast<int> (pos);
            const FloatType fract = pos - index;

            buffer(ch, i) = source(ch, index) * (1 - fract) + source(ch, index + 1) * fract;
        }

        if (ch == buffer.getNumChannels()- 1)
            seq = chSeq;
    }

    return std::make_tuple(std::move(buffer), std::move(seq));
}


} // namespace puro

