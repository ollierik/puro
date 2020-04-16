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
    std::vector<FloatType> tempWorkVector;
    std::vector<FloatType> tempAudioVector;
    std::vector<FloatType> tempEnvelopeVector;
};


template <class FloatType, class ContextType, class AudioSourceType, class EnvelopeType, class InterpolatorType>
class InterpolatingProcessor
{
public:

    InterpolatingProcessor(AudioSourceType audioSource, EnvelopeType envelope, InterpolatorType interpolator)
        : audioSource(audioSource)
        , envelopeSource(envelope)
        , interpolator(interpolator)
    {
    }

    void next(Buffer<FloatType>& output, ContextType& context)
    {
        Buffer<FloatType> audioBuffer (output, context.tempAudioVector);

        interpolator.next(audioBuffer, audioSource, context.tempWorkVector, SourceOperations::Type::replace);

        Buffer<FloatType> envelopeBuffer (audioBuffer, context.tempEnvelopeVector);
        SourceOperations::replace(envelopeBuffer, envelopeSource);

        // ensure the same size of buffers

        output.trimLengthToMatch(envelopeBuffer);
        SourceOperations::multiplyAdd(output, audioBuffer, envelopeBuffer);
    }

private:

    AudioSourceType audioSource;
    EnvelopeType envelopeSource;
    InterpolatorType interpolator;
};
