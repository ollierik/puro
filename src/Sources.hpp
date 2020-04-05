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
        for (int ch=0; ch < buffer.numChannels; ++ch)
        {
            FloatType* dst = buffer.channels[ch];
            
            for (int i=0; i<buffer.numSamples; ++i)
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
            for (int i=0; i<buffer.numSamples; ++i)
            {
                for (int ch=0; ch < buffer.numChannels; ++ch)
                {
                    const FloatType sample = sin(position);
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
                env[i] = position;
                position += increment;
            }

            Math::sin(env, buffer.numSamples);

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
        const auto sourceBufferSize = sourceBuffer.numSamples;

        // buffer will run out, trim it
        if (sourceBufferSize < index + buffer.numSamples)
        {
            buffer.trimLength(sourceBufferSize - index);
        }

        // identical channel config
        if (buffer.numChannels == sourceBuffer.numChannels)
        {
            for (int ch=0; ch<buffer.numSamples; ++ch)
            {
                FloatType* dst = buffer.channels[ch];
                FloatType* src = &sourceBuffer.channels[ch][index];

                if (opType == SourceOperations::Type::add)
                    Math::add(dst, src, buffer.numSamples);
                else
                    Math::copy(dst, src, buffer.numSamples);
            }
            index += buffer.numSamples;
        }
        // mono source, use for all channels
        else if (buffer.numChannels == 1)
        {
            FloatType* src = &sourceBuffer.channels[0][index];

            for (int ch=0; ch<buffer.numSamples; ++ch)
            {
                FloatType* dst = buffer.channels[ch];

                if (opType == SourceOperations::Type::add)
                    Math::add(dst, src, buffer.numSamples);
                else
                    Math::copy(dst, src, buffer.numSamples);
            }
            index += buffer.numSamples;
        }
        else
        {
            // channel configs not implemented
            stophere();
        }

    }

private:
    int index;
    Buffer<FloatType>& sourceBuffer;
};

