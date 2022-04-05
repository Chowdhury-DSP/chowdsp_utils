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

std::pair<juce::AudioBuffer<float>, double> AudioFileSaveLoadHelper::loadFile (const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
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
    auto* format = formatManager.findFormatForFileExtension (file.getFileExtension());
    if (format == nullptr)
    {
        juce::Logger::writeToLog ("Unable to determine audio format for file " + file.getFullPathName());
        jassertfalse;
        return false;
    }

    auto bitDepth = format->getPossibleBitDepths().getLast(); // use max bit depth by default
    std::unique_ptr<juce::AudioFormatWriter> writer (format->createWriterFor (new juce::FileOutputStream (file), sampleRate, (unsigned int) buffer.getNumChannels(), bitDepth, {}, 0));
    if (writer == nullptr)
    {
        juce::Logger::writeToLog ("Unable to create audio format writer for file " + file.getFullPathName());
        jassertfalse;
        return false;
    }

    return writer->writeFromAudioSampleBuffer (buffer, 0, buffer.getNumSamples());
}
} // namespace chowdsp
