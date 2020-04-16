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
    LinearInterpolator(FloatType rate)
        : rate(rate)
        , remainer(0)
    {}
    template <class AudioSource>
    void next(Buffer<FloatType>& dst, AudioSource& source, std::vector<FloatType> tempVector, SourceOperations::Type opType)
    {
        /**
        - Find out how many samples are needed as input to fill the given samples of output
        - Setup a masked buffer
        - Get samples from source
            - If number of samples received is less than what we wanted
            -> Find how many samples can be produced
                -> Resize buffer to flag this
        - Copy existing carryover to the beginning of the buffer
        - Find out the index k of a output sample that doesn't require info about the carryover
        - Loop to fill the output
        - Store carryover for next run
        */ 

        const int numNeeded = getNumInputSamplesNeeded(dst.size()) - numCarryover;

        Buffer<FloatType> tempBuffer (dst.getNumChannels(), numNeeded, tempVector);

        // trim to mask the space that carryover samples are copied to
        tempBuffer.trimBegin(numCarryover);
        source.next(tempBuffer, SourceOperations::Type::replace);
        tempBuffer.expandBegin(numCarryover);

        if (tempBuffer.size() < numNeeded)
        {
            const int numSamplesToProvide = getNumInputSamplesNeeded(tempBuffer.size());
            dst.trimLength(numSamplesToProvide);
        }

        const int numInputSamples = tempBuffer.size();

        for (int ch=0; ch<dst.getNumChannels(); ++ch)
        {
            FloatType* input = tempBuffer.channel(ch);
            FloatType* output = dst.channel(ch);

            // recall carryover
            for (int i=0; i<numCarryover; i++)
            {
                input[i] = carryover[ch * maxCarryover + i];
            }

            FloatType f = remainer;

            int i = 0;

            for (int outputIndex=0; outputIndex < dst.size(); ++i)
            {
                const int inputIndex = static_cast<int> (f);
                const FloatType q = f - static_cast<int> (inputIndex);

                output[outputIndex] += interpolateValue(&input[inputIndex], q);
            
                f += rate;
            }
            
            // store carryover
            const int numToStore = maxCarryover < numInputSamples ? maxCarryover : numInputSamples;

            for (int ci = 0, ii = numInputSamples - numToStore; ii < numInputSamples; ++ii, ++ci)
            {
                carryover[ch * maxCarryover + ci] = input[ii];
            }

            if (ch == dst.getNumChannels() - 1)
            {
                numCarryover = tempBuffer.size() < 2 ? tempBuffer.size() : 2;
                remainer = f - std::floor(f);
            }
        }
    }

    const FloatType rate;

private:

    int getNumInputSamplesNeeded(int outputSamples)
    {
    }
    int getNumOutputSamplesFromInputLength(int inputSamples)
    {
    }
    int getNumSamplesFromCarryover()
    {
        const int initLength = std::floor((2 - remainer)/rate);
    }


    /*
    int getNeededInputLength(int numSamples)
    {
        if (rate > 1)
        {
            return static_cast<int>(std::ceil(numSamples * rate + remainer));
        }
        else if (rate < 1)
        {
            if (initialRun)
            {
                return static_cast<int> (remainer + numSamples * rate) + 2;
            }
            else
            {
                return static_cast<int> (remainer + numSamples * rate);
            }
        }
        return 1;
    }
    */


    FloatType interpolateValue(const FloatType* xs, const FloatType q)
    {
        const FloatType x0 = xs[0];
        const FloatType x1 = xs[1];
        return (1-q) * x0 + q * x1;
    }

    int numCarryover = 0;
    FloatType remainer = 0; // (0, 1]
    static constexpr int maxCarryover = 2;
    FloatType carryover[maxCarryover * PURO_BUFFER_MAX_CHANNELS];
};
