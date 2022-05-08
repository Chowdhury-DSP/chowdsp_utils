#include <TimedUnitTest.h>
#include <test_utils.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

namespace
{
constexpr double fileSampleRate = 48000.0;
constexpr int fileNumSamples = 1000;
} // namespace

class AudioFileSaveLoadHelperTest : public TimedUnitTest
{
public:
    AudioFileSaveLoadHelperTest() : TimedUnitTest ("Audio File Save/Load Helper Test") {}

    void checkBuffersEqual (const juce::AudioBuffer<float>& actualBuffer, const juce::AudioBuffer<float>& expectedBuffer)
    {
        expectEquals (actualBuffer.getNumChannels(), expectedBuffer.getNumChannels(), "Loaded file has incorrect number of channels!");
        expectEquals (actualBuffer.getNumSamples(), expectedBuffer.getNumSamples(), "Loaded file has incorrect number of samples!");

        for (int ch = 0; ch < actualBuffer.getNumChannels(); ++ch)
        {
            for (int n = 0; n < actualBuffer.getNumSamples(); ++n)
            {
                const auto actualSample = actualBuffer.getSample (ch, n);
                const auto expectedSample = expectedBuffer.getSample (ch, n);
                expectWithinAbsoluteError (actualSample, expectedSample, 1.0e-6f, "Incorrect sample! Channel: " + juce::String (ch) + ", Sample: " + juce::String (n));
            }
        }
    }

    void saveLoadTest (const juce::AudioBuffer<float>& testBuffer)
    {
        const auto testFile = test_utils::ScopedFile { "test_file.wav" };
        chowdsp::AudioFileSaveLoadHelper saveLoadHelper;

        saveLoadHelper.saveBufferToFile (testFile.file, testBuffer, fileSampleRate);
        auto [buffer, sampleRate] = saveLoadHelper.loadFile (testFile.file);

        expectEquals (sampleRate, fileSampleRate, "Loaded file has incorrect sample rate!");
        checkBuffersEqual (buffer, testBuffer);
    }

    void failSaveTest (const juce::AudioBuffer<float>& testBuffer)
    {
        chowdsp::AudioFileSaveLoadHelper saveLoadHelper;

        {
            const auto testFile = juce::File {};
            expect (! saveLoadHelper.saveBufferToFile (testFile, testBuffer, fileSampleRate), "Saving file should fail with no file extension!");
        }

        // @TODO: figure out how to test situation where FileOutputStream cannot be created
    }

    void failLoadTest()
    {
        chowdsp::AudioFileSaveLoadHelper saveLoadHelper;
        auto [buffer, sampleRate] = saveLoadHelper.loadFile (juce::File {});

        expect (buffer.getNumChannels() == 0 && buffer.getNumSamples() == 0, "Buffer should be empty!");
        expectEquals (sampleRate, 0.0, "Sample rate should be zero!");
    }

    void runTestTimed() override
    {
        auto rand = getRandom();
        const auto testBuffer = test_utils::makeNoise (rand, fileNumSamples, rand.nextInt ({ 1, 5 }));

        beginTest ("Save/Load Test");
        saveLoadTest (testBuffer);

        beginTest ("Fail Save Test");
        failSaveTest (testBuffer);

        beginTest ("Fail Load Test");
        failLoadTest();
    }
};

static AudioFileSaveLoadHelperTest audioFileSaveLoadHelperTest;
