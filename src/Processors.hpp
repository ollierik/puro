#pragma once

template <typename FloatType>
struct GenericProcessorContext
{
    std::vector<FloatType> tempAudioBuffer;
    std::vector<FloatType> tempEnvelopeBuffer;
};


template <class FloatType, class ContextType, class AudioSourceType, class EnvelopeType>
class GenericProcessor
{
public:

    GenericProcessor(AudioSourceType audioSource, EnvelopeType envelope)
        : audioSource(audioSource)
        , envelopeSource(envelope)
    {
    }

    void next(Buffer<FloatType>& output, ContextType& context)
    {
        Buffer<FloatType> audioBuffer (output, context.tempAudioBuffer);
        SourceOperations::replace(audioBuffer, audioSource);

        Buffer<FloatType> envelopeBuffer (audioBuffer, context.tempEnvelopeBuffer);
        SourceOperations::replace(envelopeBuffer, envelopeSource);

        // ensure the same size of buffers
        output.trimLengthToMatch(envelopeBuffer);

        SourceOperations::multiplyAdd(output, audioBuffer, envelopeBuffer);
    }

private:

    AudioSourceType audioSource;
    EnvelopeType envelopeSource;
};


/** Interpolating */

template <typename FloatType>
struct InterpolatingProcessorContext
{
    std::vector<FloatType> tempInterpBuffer;
    std::vector<FloatType> tempAudioBuffer;
    std::vector<FloatType> tempEnvelopeBuffer;
};


template <class FloatType, class ContextType, class AudioSourceType, class EnvelopeType, class InterpolatorType>
class InterpolatingProcessor
{
public:

    InterpolatingProcessor(FloatType playbackSpeed, AudioSourceType audioSource, EnvelopeType envelope)
        : rate (1 / playbackSpeed)
        , audioSource(audioSource)
        , envelopeSource(envelope)
    {
    }

    void next(Buffer<FloatType>& output, ContextType& context)
    {
        Buffer<FloatType> audioBuffer (output, context.tempAudioBuffer);
        SourceOperations::replace(audioBuffer, audioSource);

        Buffer<FloatType> envelopeBuffer (audioBuffer, context.tempEnvelopeBuffer);
        SourceOperations::replace(envelopeBuffer, envelopeSource);

        // ensure the same size of buffers
        output.trimLengthToMatch(envelopeBuffer);


        // TODO rounding
        int lengthForRate = output.size() * rate;
        Buffer<FloatType> interpBuffer (output.getNumChannels(), lengthForRate, context.tempInterpBuffer);

        // copy to interpolation buffer
        SourceOperations::multiplyAdd(interpBuffer, audioBuffer, envelopeBuffer);

        SourceOperations::add(output, interpBuffer, audioSource);
    }

private:

    const float rate; // 1 / speed

    AudioSourceType audioSource;
    EnvelopeType envelopeSource;
    InterpolatorType interpolator;
};
