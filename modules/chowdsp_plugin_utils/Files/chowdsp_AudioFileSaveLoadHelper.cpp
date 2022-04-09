#include "chowdsp_AudioFileSaveLoadHelper.h"

namespace chowdsp
{
AudioFileSaveLoadHelper::AudioFileSaveLoadHelper()
{
    formatManager.registerBasicFormats();
}

AudioFileSaveLoadHelper::~AudioFileSaveLoadHelper()
{
    formatManager.clearFormats();
}

std::unique_ptr<juce::AudioFormatReader> AudioFileSaveLoadHelper::createReaderFor (const juce::File& file)
{
    return std::unique_ptr<juce::AudioFormatReader> (formatManager.createReaderFor (file));
}

std::unique_ptr<juce::AudioFormatWriter> AudioFileSaveLoadHelper::createWriterFor (const juce::File& file, const AudioFileWriterParams& params)
{
    auto* format = formatManager.findFormatForFileExtension (file.getFileExtension());
    if (format == nullptr)
    {
        juce::Logger::writeToLog ("Unable to determine audio format for file " + file.getFullPathName());
        jassertfalse;
        return nullptr;
    }

    auto audioFileStream = std::make_unique<juce::FileOutputStream> (file);
    auto bitDepth = params.bitsPerSample > 0 ? params.bitsPerSample : format->getPossibleBitDepths().getLast();

    if (auto writer = std::unique_ptr<juce::AudioFormatWriter> (format->createWriterFor (audioFileStream.get(), params.sampleRateToUse, params.numberOfChannels, bitDepth, params.metadataValues, params.qualityOptionIndex)))
    {
        // the audio format writer now owns the file stream pointer, so let's release it here to avoid a double-delete
        auto* releasedFileStream = audioFileStream.release();
        juce::ignoreUnused (releasedFileStream);
        return writer;
    }

    juce::Logger::writeToLog ("Unable to create audio format writer for file " + file.getFullPathName());
    jassertfalse;
    return nullptr;
}

std::pair<juce::AudioBuffer<float>, double> AudioFileSaveLoadHelper::loadFile (const juce::File& file)
{
    auto reader = createReaderFor (file);
    if (reader == nullptr)
    {
        juce::Logger::writeToLog ("Unable to create audio format reader for file " + file.getFullPathName());
        jassertfalse;
        return {};
    }

    const auto sampleRate = reader->sampleRate;
    const auto numChannels = (int) reader->numChannels;
    const auto numSamples = (int) reader->lengthInSamples;
    juce::AudioBuffer<float> buffer (numChannels, numSamples);

    if (! reader->read (buffer.getArrayOfWritePointers(), numChannels, 0, numSamples))
    {
        jassertfalse; // there was some problem reading the samples from the audio file into the buffer!
        return {};
    }

    return { std::move (buffer), sampleRate };
}

bool AudioFileSaveLoadHelper::saveBufferToFile (const juce::File& file, const juce::AudioBuffer<float>& buffer, double sampleRate)
{
    if (auto writer = createWriterFor (file, AudioFileWriterParams { sampleRate, (unsigned int) buffer.getNumChannels() }))
        return writer->writeFromAudioSampleBuffer (buffer, 0, buffer.getNumSamples());

    return false;
}
} // namespace chowdsp
