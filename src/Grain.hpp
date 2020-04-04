#pragma once

template <typename FloatType>
struct EnvelopeProcessorContext
{
    std::vector<FloatType> tempAudioBuffer;
    std::vector<FloatType> tempEnvelopeBuffer;
};


template <class FloatType, class ContextType, class AudioSourceType, class EnvelopeType>
class EnvelopeProcessor
{
public:

    EnvelopeProcessor(AudioSourceType audioSource, EnvelopeType envelope)
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
