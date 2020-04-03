#pragma once

/** Simple wrapper around audio data with helper functions for accessing and debug checks. */
template <class FloatType>
struct Buffer
{
    Buffer(FloatType* data, int numChannels, int numSamples)
        : data(data), numChannels(numChannels), numSamples(numSamples)
    {}

    FloatType* data;
    const int numChannels;
    const int numSamples;

    FloatType& operator[] (int i)
    {
        stopif(i < 0 || i >= numSamples * numChannels, "index out of bounds");
        return data[i];
    }

    FloatType* getPtr(int ch, int i)
    {
        stopif(i < 0 || i >= numSamples, "index out of bounds");
        stopif(ch < 0 || i >= numChannels, "channel out of bounds");
        return &data[numSamples*ch + i];
    }

    FloatType& get(int ch, int i)
    {
        return *getPtr(ch, i);
    }

    FloatType* getChannelData(int ch)
    {
        return getPtr(ch, 0);
    }

    /** Set range of values to a constant for a single channel */
    void setChannelTo(FloatType value, int ch, int i0, int num)
    {
        stopif(num < 0, "num is negative");
        stopif(i0 + num > numSamples, "end of range out of bounds");
        Math::set(getPtr(ch, i0), num, value);
    }

    /** Set range of values to a constant for all channels */
    void setAllChannelsTo(FloatType value, int i0, int num)
    {
        stopif(num < 0, "num is negative");
        stopif(i0 + num > numSamples, "end of range out of bounds");

        for (int ch = 0; ch < numChannels; ++ch)
        {
            Math::set(getPtr(ch, i0), num, value);
        }
    }
};
