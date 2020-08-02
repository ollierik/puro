#pragma once

namespace puro {

template <class FloatType, int numberOfChannels>
struct RingBuffer
{
    // member fields
    int numSamples;
    int writeIndex = 0;
    std::array<FloatType*, numberOfChannels> channelPtrs;

    // template arg broadcasts
    typedef FloatType value_type;
    static constexpr int num_channels = numberOfChannels;

    // getters
    bool isInvalid() const noexcept { return numSamples <= 0; }
    int length() const noexcept { return numSamples; }
    constexpr int getNumChannels() const noexcept { return num_channels; } // some more advanced class may want to redefine this

    FloatType* channel(int ch) const noexcept
    {
        errorif(ch < 0 || ch >= num_channels, "channel out of range");
        return channelPtrs[ch];
    }

    // constructors

    RingBuffer() : numSamples(0) {} // invalid Buffer

    RingBuffer (int numSamples) noexcept
        : numSamples(numSamples)
    {}

    /** Buffer from raw allocated memory.
        Provided data is expected to be able to hold (numSamples * numChannels) of data */
    RingBuffer (int numSamples, FloatType* data) noexcept
        : numSamples(numSamples)
    {
        for (int ch = 0; ch < num_channels; ++ch)
            channelPtrs[ch] = &data[ch * numSamples];
    }

    /** Buffer from raw allocated memory. Provided array should contain the per-channel pointers. */
    RingBuffer (int numSamples, std::array<FloatType, numberOfChannels> data) noexcept
        : numSamples(numSamples)
    {
        for (int ch = 0; ch < data.size(); ++ch)
            channelPtrs[ch] = &data[ch];
    }
};

template <typename RingBufferType>
RingBufferType ringbuffer_advance_write_index(RingBufferType ringBuffer, int amount)
{
    ringBuffer.writeIndex += amount;

    if (ringBuffer.writeIndex > ringBuffer.length())
        ringBuffer.writeIndex -= ringBuffer.length();

    return ringBuffer;
}

template <typename RingBufferType, typename BufferType>
RingBufferType ringbuffer_write_and_advance(RingBufferType ringBuffer, BufferType input)
{
    errorif(ringBuffer.getNumChannels() != input.getNumChannels(), "channel configs don't match");
    errorif(ringBuffer.length() < input.length(), "input shouldn't be longer than ring buffer");

    const int overflow = ringBuffer.writeIndex + input.length() - ringBuffer.length();

    // ring buffer wraps around
    if (overflow > 0)
    {
        const int length = input.length() - overflow;

        for (int ch=0; ch<ringBuffer.getNumChannels(); ++ch)
        {
            math::copy(&ringBuffer.channel(ch)[index], input.channel(ch), length);
            math::copy(&ringBuffer.channel(ch)[0], &input.channel(ch)[length], overflow);
        }
    }
    else
    {
        for (int ch=0; ch<ringBuffer.getNumChannels(); ++ch)
        {
            math::copy(&ringBuffer.channel(ch)[index], input.channel(ch), input.size());
        }
    }
    ringbuffer_advance_write_index(ringBuffer, input.length());
    return ringBuffer;
}




} // namespace puro

