#pragma once

namespace puro {


template <int increment = 1>
struct IndexSequence
{
    IndexSequence(int val) : value(val) {}

    static constexpr int increment = increment;

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
        for (int toCh=0; toCh < numChannels; ++toCh)
        {
            const auto coef = coeffs(fromCh, toCh);
            math::multiply_add(dst.channel(toCh), src.channel(fromCh), coef, dst.size());
        }
    }
}

///======================================================================================


template <typename BufferType>
void noise_fill(BufferType buffer)
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
}

template <typename BufferType>
void constant_fill(BufferType buffer, typename BufferType::value_type value)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        math::set(buffer.channel(ch), value, buffer.size());
    }
}

template <typename BufferType, typename ValueType>
void linspace_fill(BufferType buffer, ValueType start, const ValueType increment)
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
SeqType envl_halfcos_fill(BufferType buffer, SeqType seq)
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

    return seq;
}

template <typename FloatType>
Sequence<FloatType> envl_hann_create_seq(int lengthInSamples, bool symmetric = true)
{
    const FloatType div = static_cast<FloatType>(lengthInSamples) + (symmetric ? 1 : 0);
    const FloatType val = 2 * math::pi<FloatType>() / div;

    return { val, val };
}

template <typename BufferType, typename SeqType>
SeqType envl_hann_fill(BufferType buffer, SeqType seq)
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

    return seq;
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


template <typename FloatType>
int interp_num_samples_available(int length, FloatType position, const FloatType rate, const int interp_order)
{
    return (int)std::ceil((static_cast<FloatType>(length-interp_order) - position)/rate);
}

template <typename BufferType, typename SeqType>
BufferType interp_crop_buffer(BufferType buffer, int samplesAvailable, SeqType seq, const int interpOrder)
{
    const int numAvailable = interp_num_samples_available(samplesAvailable, seq.value, seq.increment, interpOrder);

    if (numAvailable < buffer.size())
        buffer = puro::trimmed_length(buffer, numAvailable);

    return buffer;
}

/** Assumes that the source buffer can provide all the required samples, i.e. doesn't do bound checking.
    Buffer should be cropped for example with buffer_crop_for_interp before-hand. */
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

