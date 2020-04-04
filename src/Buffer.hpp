#pragma once

/** Simple wrapper around audio buffer data with helper functions for accessing and debug checks. */
template <class FloatType>
struct Buffer
{
    std::array<FloatType*, 2> channels; // TODO stereo only for now
    int numChannels;
    int numSamples;

    //////////////////

    Buffer(int numChannels, int numSamples)
        : numChannels(numChannels), numSamples(numSamples)
    {}

    Buffer(const std::array<FloatType*, 2>& channels, int numChannels, int numSamples)
        : channels(channels), numChannels(numChannels), numSamples(numSamples)
    {
    }

    /** Return a buffer that can accomodate provided buffer, fitted into the provided vector
        If vector can't fit the created buffer, it will be resized. */

    Buffer<FloatType> (Buffer<FloatType>& other, std::vector<FloatType>& vector)
        : numChannels(other.numChannels), numSamples(other.numSamples)
    {
        // resize if needed
        if (vector.size() < numSamples * numChannels)
        {
            vector.resize(numSamples * numChannels);
        }

        for (int ch=0; ch<numChannels; ++ch)
            buffer.channels[ch] = &vector[ch * numSamples];

        return buffer;
    }

    Buffer<FloatType>& clip(int offset, int n)
    {
        Buffer clipped(numChannels, n);
        for (int ch=0; ch<numChannels; ++ch)
            clipped.channels[ch] = &channels[ch][offset];

        return clipped;
    }


#if 0

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
#endif
};

class SourceOperations
{
    enum class Type { replace, add };

    template <typename FloatType, class SourceType>
    static void replace(Buffer<FloatType>& dst, SourceType& source)
    {
        source.next(dst, Type::replace);
    }

    template <typename FloatType>
    static void multiplyAdd(Buffer<FloatType>& dst, const Buffer<FloatType>& src1, const Buffer<FloatType>& src2)
    {
        stopif(dst.numChannels != src1.numChannels, "Number of channels doesn't match");
        stopif(dst.numChannels != src2.numChannels, "Number of channels doesn't match");

        // pick the smallest n
        int n = std::min(std::min(dst.numSamples, src1.numSamples), src2.numSamples);

        for (int ch=0; ch<dst.numChannels; ++c)
        {
            Math::multiplyAdd(dst.channels[ch], src1.numChannels[ch], src2.numChannels[ch]);
        }
    }
};
