#pragma once

// as macro for now, should be refactored
#define PURO_BUFFER_MAX_CHANNELS 2

/** Simple wrapper around audio buffer data with helper functions for accessing and debug checks. */
template <class FloatType>
class Buffer
{

public:

    //////////////////

    Buffer (int numChannels, int numSamples)
        : numChannels(numChannels) , numSamples(numSamples)
    {}

    Buffer (const std::array<FloatType*, PURO_BUFFER_MAX_CHANNELS>& channels, int numChannels, int numSamples)
        : channels(channels) , numChannels(numChannels), numSamples(numSamples)
    {}

    /** Create a buffer with given shape, with channels fitted into the provided vector.
        If vector can't fit the created buffer, it will be resized. */
    Buffer (int numChannels, int numSamples, std::vector<FloatType>& vector)
        : numChannels(numChannels), numSamples(numSamples)
    {
        // resize if needed
        if (vector.size() < numSamples * numChannels)
        {
            vector.resize(numSamples * numChannels);
        }

        for (int ch=0; ch<numChannels; ++ch)
            channels[ch] = &vector[ch * numSamples];
    }

    /** Create a buffer that has same shape as the provided buffer, with channels fitted into the provided vector.
        If vector can't fit the created buffer, it will be resized. */
    Buffer (Buffer<FloatType>& other, std::vector<FloatType>& vector)
        : numChannels(other.numChannels), numSamples(other.numSamples)
    {
        errorif(other.numChannels > PURO_BUFFER_MAX_CHANNELS, "TODO implement this in a more general way");

        // resize if needed
        if (vector.size() < numSamples * numChannels)
        {
            vector.resize(numSamples * numChannels);
        }

        for (int ch=0; ch<numChannels; ++ch)
            channels[ch] = &vector[ch * numSamples];
    }

    FloatType* channel(int ch) const
    {
        errorif(ch < 0 || ch > numChannels, "channel out of range");
        errorif(ch >= PURO_BUFFER_MAX_CHANNELS, "requested channel above maxmimum");
        return channels[ch];
    }

    int getNumChannels() { return numChannels; };
    int size() { return numSamples; };

    const std::pair<int, int> shape() const { return { numChannels, numSamples }; };

    void trimBegin(int offset)
    {
        errorif(offset < 0 || offset > numSamples, "offset out of bounds");

        numSamples -= offset;

        for (int ch=0; ch<numChannels; ++ch)
            channels[ch] = &channels[ch][offset];
    }

    void trimLength(int newLength)
    {
        errorif(newLength < 0 || newLength > numSamples, "new length out of bounds");

        numSamples = newLength;
    }

    void trimLengthToMatch(Buffer<FloatType>& other)
    {
        if (other.numSamples < numSamples)
        {
            numSamples = other.numSamples;
        }
    }

    bool dimensionsMatch(const Buffer<FloatType>& other)
    {
        return (size() == other.size() && getNumChannels() == other.getNumChannels());
    }


private:

    std::array<FloatType*, PURO_BUFFER_MAX_CHANNELS> channels;
    int numChannels;
    int numSamples;
};


struct SourceOperations
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
        errorif(! (dst.shape() == src1.shape()), "dst and src1 buffer dimensions don't match");
        errorif(! (dst.shape() == src2.shape()), "dst and src2 buffer dimensions don't match");

        for (int ch=0; ch<dst.getNumChannels(); ++ch)
        {
            /*
            auto* d = dst.getChannel(ch);
            const auto* s1 = src1.getConstChannel(ch);
            const auto* s2 = src2.getConstChannel(ch);
            Math::multiplyAdd<FloatType>(d, s1, s2, dst.size());
            */
            Math::multiplyAdd<FloatType>(dst.channel(ch), src1.channel(ch), src2.channel(ch), dst.size());
            //Math::multiplyAdd<FloatType>(dst.getChannel(ch), src1.getConstChannel(ch), src2.getConstChannel(ch), dst.size());
        }
    }
};
