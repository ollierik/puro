#pragma once

namespace puro {

/*
template <int increment = 1>
struct IndexSequence
{
    IndexSequence(int val) noexcept : value(val) {}

    static constexpr int increment = increment;

    int value;
};
*/



template <typename FloatType>
struct Sequence
{
    typedef FloatType value_type;

    Sequence(FloatType val, FloatType inc) : value(val), increment(inc) {}

    FloatType value;
    FloatType increment;
};

template <typename SequenceType>
SequenceType sequence_increment(SequenceType seq)
{
    seq.value += seq.increment;
    return seq;
}


} // namespace puro
