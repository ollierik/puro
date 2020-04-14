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

    InterpolatingProcessor(AudioSourceType audioSource, EnvelopeType envelope, InterpolatorType interpolator)
        : audioSource(audioSource)
        , envelopeSource(envelope)
        , interpolator(interpolator)
    {
    }

    void next(Buffer<FloatType>& output, ContextType& context)
    {
        const int numInputNeeded = interpolator.getNeededInputLength(output.size());

        Buffer<FloatType> audioBuffer (output.getNumChannels(), numInputNeeded, context.tempAudioBuffer);
        SourceOperations::replace(audioBuffer, audioSource);

        Buffer<FloatType> envelopeBuffer (audioBuffer, context.tempEnvelopeBuffer);
        SourceOperations::replace(envelopeBuffer, envelopeSource);

        Buffer<FloatType> interpBuffer (audioBuffer, context.tempInterpBuffer);

        // copy to the padded interpolation buffer
        SourceOperations::multiply(interpBuffer, audioBuffer, envelopeBuffer);

        SourceOperations::add(output, interpBuffer, interpolator);

        // ensure the same size of buffers
        //output.trimLengthToMatch(envelopeBuffer);
    }

private:

    AudioSourceType audioSource;
    EnvelopeType envelopeSource;
    InterpolatorType interpolator;
};
