#include <CatchUtils.h>
#include <test_utils.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

namespace
{
constexpr double fileSampleRate = 48000.0;
constexpr int fileNumSamples = 1000;
} // namespace

static void checkBuffersEqual (const juce::AudioBuffer<float>& actualBuffer, const juce::AudioBuffer<float>& expectedBuffer)
{
    REQUIRE_MESSAGE (actualBuffer.getNumChannels() == expectedBuffer.getNumChannels(), "Loaded file has incorrect number of channels!");
    REQUIRE_MESSAGE (actualBuffer.getNumSamples() == expectedBuffer.getNumSamples(), "Loaded file has incorrect number of samples!");

    for (int ch = 0; ch < actualBuffer.getNumChannels(); ++ch)
    {
        for (int n = 0; n < actualBuffer.getNumSamples(); ++n)
        {
            const auto actualSample = actualBuffer.getSample (ch, n);
            const auto expectedSample = expectedBuffer.getSample (ch, n);
            REQUIRE_MESSAGE (actualSample == Catch::Approx { expectedSample }.margin (1.0e-6f), "Incorrect sample! Channel: " + juce::String (ch) + ", Sample: " + juce::String (n));
        }
    }
}

TEST_CASE ("Audio File Save/Load Helper Test", "[plugin][utilities]")
{
    const auto testBuffer = test_utils::makeNoise (fileNumSamples, test_utils::RandomIntGenerator { 1, 5 }());

    SECTION ("Save/Load Test")
    {
        const auto testFile = test_utils::ScopedFile { "test_file.wav" };
        chowdsp::AudioFileSaveLoadHelper saveLoadHelper;

        saveLoadHelper.saveBufferToFile (testFile.file, testBuffer.toAudioBuffer(), fileSampleRate);
        auto [buffer, sampleRate] = saveLoadHelper.loadFile (testFile.file);

        REQUIRE_MESSAGE (juce::approximatelyEqual (sampleRate, fileSampleRate), "Loaded file has incorrect sample rate!");
        checkBuffersEqual (buffer, testBuffer.toAudioBuffer());
    }

    SECTION ("Fail Save Test")
    {
        chowdsp::AudioFileSaveLoadHelper saveLoadHelper;

        {
            const auto testFile = juce::File {};
            REQUIRE_MESSAGE (! saveLoadHelper.saveBufferToFile (testFile, testBuffer.toAudioBuffer(), fileSampleRate), "Saving file should fail with no file extension!");
        }

        // @TODO: figure out how to test situation where FileOutputStream cannot be created
    }

    SECTION ("Fail Load Test")
    {
        chowdsp::AudioFileSaveLoadHelper saveLoadHelper;
        auto [buffer, sampleRate] = saveLoadHelper.loadFile (juce::File {});

        REQUIRE_MESSAGE ((buffer.getNumChannels() == 0 && buffer.getNumSamples() == 0), "Buffer should be empty!");
        REQUIRE_MESSAGE (juce::exactlyEqual (sampleRate, 0.0), "Sample rate should be zero!");
    }
}
