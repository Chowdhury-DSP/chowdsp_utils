#pragma once

namespace chowdsp
{
/** Helper class for interfacing between AudioBuffers and files. */
class AudioFileSaveLoadHelper
{
public:
    AudioFileSaveLoadHelper();
    ~AudioFileSaveLoadHelper();

    struct AudioFileWriterParams
    {
        double sampleRateToUse = 48000.0;
        unsigned int numberOfChannels = 2;
        int bitsPerSample = 0;
        juce::StringPairArray metadataValues {};
        int qualityOptionIndex = 0;
    };

    /**
     * Loads the given file into an AudioBuffer, and returns
     * the buffer, along with the file sample rate
     */
    std::pair<juce::AudioBuffer<float>, double> loadFile (const juce::File& file);

    /**
     * Saves an AudioBuffer to a file.
     *
     * Returns true if the operation succeeded.
     */
    bool saveBufferToFile (const juce::File& file, const juce::AudioBuffer<float>& buffer, double sampleRate);

    /** Creates an audio format reader for the given file */
    std::unique_ptr<juce::AudioFormatReader> createReaderFor (const juce::File& file);

    /** Creates an audio format writer for the given file */
    std::unique_ptr<juce::AudioFormatWriter> createWriterFor (const juce::File& file, const AudioFileWriterParams& params);

    juce::AudioFormatManager formatManager;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFileSaveLoadHelper)
};

/** Shared resource pointer type for chowdsp::AudioFileSaveLoadHelper */
using SharedAudioFileSaveLoadHelper = juce::SharedResourcePointer<AudioFileSaveLoadHelper>;
} // namespace chowdsp
