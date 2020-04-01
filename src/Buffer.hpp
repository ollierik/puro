#pragma once

/** Simple wrapper around audio data with helper functions for accessing it in concise manner. */
template <class FloatType>
class Buffer
{
public:
    FloatType* data;
    const int numChannels;
    const int length;

    FloatType* getChannelData(int channel) { return data[length*numChannels]; }
};
