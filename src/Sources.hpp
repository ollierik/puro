#pragma once

/////////////////////////////////////////////
// Audio sources
/////////////////////////////////////////////

template <typename FloatType>
class ConstSource
{
public:

    void next(Buffer<FloatType>& buffer, SourceOperations::Type opType)
    {
        for (int ch=0; ch < buffer.getNumChannels(); ++ch)
        {
            FloatType* dst = buffer.channel(ch);
            
            for (int i=0; i<buffer.size(); ++i)
            {
                if (opType == SourceOperations::Type::add) dst[i] += 1.0;
                else dst[i] = 1.0;
            }
        }
    }
};

template <typename FloatType>
class NoiseSource
{
public:
    void next(Buffer<FloatType>& buffer, SourceOperations::Type opType)
    {
        for (int ch=0; ch < buffer.numChannels; ++ch)
        {
            FloatType* dst = buffer.channels[ch];
            
            for (int i=0; i<buffer.numSamples; ++i)
            {
                const FloatType coef = static_cast<FloatType> (1) / static_cast<FloatType> (RAND_MAX/2);
                const FloatType r = static_cast<FloatType> (std::rand()) * coef - 1;

                if (opType == SourceOperations::Type::add) dst[i] += r;
                else dst[i] = r;
            }
        }
    }
};

/////////////////////////////////////////////
// Audio sources
/////////////////////////////////////////////

template <class FloatType>
class SineEnvelope
{
public:

    SineEnvelope(int lengthInSamples)
        : increment(Math::pi<FloatType>() / static_cast<FloatType>(lengthInSamples-1))
        , position(increment)
    {
    }

    void next(Buffer<FloatType>& buffer, SourceOperations::Type opType)
    {
        // do sample-wise
        if (opType == SourceOperations::Type::add)
        {
            for (int i=0; i<buffer.size(); ++i)
            {
                for (int ch=0; ch < buffer.getNumChannels(); ++ch)
                {
                    const FloatType sample = sin(position);
                    buffer.channel(ch)[i] = sample;
                }

                position += increment;
            }
        }
        else
        {
            // do vectorised
            FloatType* env = buffer.channel(0);

            for (int i=0; i<buffer.size(); ++i)
            {
                env[i] = position;
                position += increment;
            }

            Math::sin(env, buffer.size());

            for (int ch=1; ch < buffer.getNumChannels(); ++ch)
            {
                FloatType* dst = buffer.channel(ch);
                Math::copy(dst, env, buffer.size());
            }
        }
    }

private:

    const FloatType increment;
    FloatType position;
};



template <class FloatType>
class HannEnvelope
{
public:

    HannEnvelope(int lengthInSamples)
        : increment(2 * Math::pi<FloatType>() / static_cast<FloatType>(lengthInSamples-1))
        , position(increment)
    {
    }


    void next(Buffer<FloatType>& buffer, SourceOperations::Type opType)
    {
        // do sample-wise
        if (opType == SourceOperations::Type::add)
        {
            for (int i=0; i<buffer.numSamples; ++i)
            {
                for (int ch=0; ch < buffer.numChannels; ++ch)
                {
                    const FloatType sample = (1 - cos(position)) / 2;
                    buffer.channels[ch][i] = sample;
                }

                position += increment;
            }
        }
        else
        {
            // do vectorised
            FloatType* env = buffer.channels[0];

            for (int i=0; i<buffer.numSamples; ++i)
            {
                const FloatType sample = (1 - cos(position)) / 2;
                env[i] = sample;
                position += increment;
            }

            for (int ch=1; ch < buffer.numChannels; ++ch)
            {
                FloatType* dst = buffer.channels[ch];
                Math::copy(dst, env, buffer.numSamples);
            }
        }
    }

private:

    const FloatType increment;
    FloatType position;
};



template <typename FloatType>
class AudioBufferSource
{
public:
    AudioBufferSource(Buffer<FloatType>& fileBuffer, int startIndex)
        : sourceBuffer(fileBuffer)
        , index(startIndex)
    {
    }

    void next(Buffer<FloatType>& buffer, SourceOperations::Type opType)
    {
        // buffer will run out, trim it
        if (sourceBuffer.size()< index + buffer.size())
        {
            buffer.trimLength(sourceBuffer.size() - index);
        }

        // identical channel config
        if (buffer.getNumChannels() == sourceBuffer.getNumChannels())
        {
            for (int ch=0; ch<buffer.getNumChannels(); ++ch)
            {
                FloatType* dst = buffer.channel(ch);
                FloatType* src = &sourceBuffer.channel(ch)[index];

                if (opType == SourceOperations::Type::add)
                    Math::add<FloatType>(dst, src, buffer.size());
                else
                    Math::copy<FloatType>(dst, src, buffer.size());
            }
            index += buffer.size();
        }
        // mono source, use for all channels
        else if (sourceBuffer.getNumChannels() == 1)
        {
            FloatType* src = &sourceBuffer.channel(0)[index];

            for (int ch=0; ch<buffer.getNumChannels(); ++ch)
            {
                FloatType* dst = buffer.channel(ch);

                if (opType == SourceOperations::Type::add)
                    Math::add(dst, src, buffer.size());
                else
                    Math::copy(dst, src, buffer.size());
            }
            index += buffer.size();
        }
        else
        {
            errorif(true, "channel config combination not implemented");
        }

    }

private:
    int index;
    Buffer<FloatType>& sourceBuffer;
};

/////////////////////////////////////////////
// Interpolators
/////////////////////////////////////////////

template <typename FloatType>
class LinearInterpolator
{
public:
    LinearInterpolator(FloatType rate) : rate(rate)
    {}

    template <class AudioSource>
    void next(Buffer<FloatType>& dst, AudioSource& source, std::vector<FloatType> tempVector, SourceOperations::Type opType)
    {
        /*
        - Find the index of last sample we need
        - Find how many input samples are needed to reach it

        - Request samples from source
            - If number of samples received is less than what we wanted
            -> Find how many samples can be produced
                -> Resize buffer to flag this
        - If needed, copy carryover
        - Loop
        */

        const int numToRequest = getNumSamplesToRequest(dst.size());
        const int numExpected = numToRequest + numCarryover;

        Buffer<FloatType> tempBuffer (dst.getNumChannels(), numExpected, tempVector);

        if (numCarryover > 0)
        {
            // trim to mask the space that carryover samples are copied to
            tempBuffer.trimBegin(numCarryover);
            source.next(tempBuffer, SourceOperations::Type::replace);
            tempBuffer.expandBegin(numCarryover);
        }
        else
        {
            source.next(tempBuffer, SourceOperations::Type::replace);
        }

        if (tempBuffer.size() < numExpected)
        {
            const int numSamplesToProvide = getNumSamplesToProvide(tempBuffer.size());
            dst.trimLength(numSamplesToProvide);
        }

        // restore carryover from state if needed
        if (numCarryover > 0)
        {
            for (int ch=0; ch < dst.getNumChannels(); ++ch)
                for (int i=0; i<numCarryover; ++i)
                    restoreCarryoverSample(tempBuffer, ch, i);
        }

        const int numInputSamples = tempBuffer.size();

        for (int ch=0; ch < dst.getNumChannels(); ++ch)
        {
            FloatType* input = tempBuffer.channel(ch);
            FloatType* output = dst.channel(ch);

            FloatType pos = continueAt - static_cast<FloatType> (inputPosition);

            for (int outputIndex = 0; outputIndex < dst.size(); outputIndex)
            {
                const int inputIndex = static_cast<int> (pos);
                const FloatType q = pos - static_cast<FloatType> (inputIndex);

                output[outputIndex] += interpolateValue(&input[inputIndex], q);
                pos += rate;
            }

            if (ch == dst.getNumChannels() - 1)
            {
                continueAt = pos + inputPosition;
                inputPosition += numInputSamples;
            }
        }

        updateCarryover(tempBuffer);

    }

    const FloatType rate;

private:

    int getNumSamplesToRequest(int numOutput)
    {
        const int highest = static_cast<int> (std::ceil(continueAt + (numOutput-1) * rate));
        const int diff = highest - inputPosition;
        return diff;
    }

    int getNumSamplesToProvide(int numInput)
    {
        const FloatType range = static_cast<FloatType>(numInput + inputPosition) - (continueAt - rate);
        const int n = static_cast<int> (std::ceil(range / rate));
        return n;
    }

    void updateCarryover(Buffer<FloatType>& inputBuffer)
    {
        const int lowestNeeded = static_cast<int> (std::floor(continueAt));
        const int diff = inputPosition - lowestNeeded + (maxCarryover-1);
        numCarryover = diff > maxCarryover ? maxCarryover : diff;

        for (int ch=0; ch<inputBuffer.getNumChannels(); ++ch)
            for (int i = 0; i < numCarryover; i++)
                storeCarryoverSample(inputBuffer, ch, i);

    }

    void restoreCarryoverSample(Buffer<FloatType>& inputBuffer, int ch, int i)
    {
        const FloatType sample = carryover[ch * maxCarryover + i];
        inputBuffer.channel(ch)[i] = sample;
    }

    void storeCarryoverSample(Buffer<FloatType>& inputBuffer, int ch, int i)
    {
        const auto first = inputBuffer.size() - numCarryover;
        const FloatType sample = inputBuffer.channel(ch)[first + i];
        carryover[ch * maxCarryover + i] = sample;
    }

    FloatType interpolateValue(const FloatType* xs, const FloatType q)
    {
        const FloatType x0 = xs[0];
        const FloatType x1 = xs[1];
        return (1-q) * x0 + q * x1;
    }

    int inputPosition = -1;
    FloatType continueAt = 0;
    int numCarryover = 0;

    static constexpr int maxCarryover = 2;
    FloatType carryover[maxCarryover * PURO_BUFFER_MAX_CHANNELS];
};
