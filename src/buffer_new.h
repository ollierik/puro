#pragma once

#include <cstdint>
#include <array>
#include "defs.hpp"

struct SoundRanges
{
    int offset;
    int remaining;
};

template <typename F, int numChannels=1>
struct Buffer
{
    int n;
    std::array<F*, numChannels> channels;

    Buffer() : n(0), channels({nullptr}) {} // invalid Buffer
    Buffer(int length, std::array<F*, numChannels> channelData) : n(length), channels(channelData) {}

    /** Create a buffer that has same shape as the provided buffer, with channels fitted into the provided vector.
        If vector can't fit the created buffer, it will be resized. */
    Buffer (Buffer& other, std::vector<F>& vector) : n(other.n)
    {
        errorif(n == 0, "Length of the other buffer is 0");

        // resize if needed
        if (vector.size() < n * numChannels)
            vector.resize(n * numChannels);

        for (int ch=0; ch<numChannels; ++ch)
            channels[ch] = &vector[ch * n];
    }

    constexpr int getNumChannels() const { return numChannels; };

    F* ch(int ch) const
    {
        errorif(ch < 0 || ch > numChannels, "channel index out of range");
        return channels[ch];
    }

    void trimBegin(int offset)
    {
        errorif(offset < 0 || offset > n, "offset out of bounds");

        for (int ch=0; ch<numChannels; ++ch)
            channels[ch] = &channels[ch][offset];

        n -= offset;
    }

    void trimLength(int newLength)
    {
        errorif(newLength < 0 || newLength > n, "new length out of bounds");
        n = newLength;
    }


    size_t size() const { return n; }
};


template <typename BufferType>
BufferType crop_buffer(const BufferType& buffer, const SoundRanges& ranges)
{
    // no operations needed for this block
    if (ranges.offset >= buffer.size())
    {
        return BufferType();
    }

    BufferType cropped = buffer;

    if (ranges.offset > 0)
        cropped.trimBegin(ranges.offset);

    // restrict range if the sound object should terminate this block
    if (ranges.remaining < buffer.n)
        cropped.trimLength(ranges.remaining);

    return cropped;
}

void advance_ranges(SoundRanges& ranges, int numSamplesRequested)
{
    ranges.offset = ranges.offset > 0 ? ranges.offset - numSamplesRequested : 0;
    ranges.remaining -= numSamplesRequested;
}
