#include <test_utils.h>
#include <TimedUnitTest.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

namespace
{
const juce::StringArray testStrings { "TEST_1", "TEST_2", "TEST_3" };
} // namespace

class FileListenerTest : public TimedUnitTest
{
public:
    FileListenerTest() : TimedUnitTest ("File Listener Test")
    {
    }

    void fileChangeTest()
    {
        test_utils::ScopedFile testFile ("test_file.txt");
        testFile.file.replaceWithText (testStrings[0]);

        struct TestListener : private chowdsp::FileListener
        {
            juce::String currentFileText {};

            explicit TestListener (const juce::File& f) : chowdsp::FileListener (f, 1)
            {
                listenerFileChanged();
            }

            void listenerFileChanged() final
            {
                currentFileText = getListenerFile().loadFileAsString();
            }
        } testListener (testFile.file);

        expectEquals (testListener.currentFileText, testStrings[0], "Initial file text incorrect!");

        juce::MessageManager::getInstance()->runDispatchLoopUntil (1500);

        testFile.file.replaceWithText (testStrings[1]);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (1500);
        expectEquals (testListener.currentFileText, testStrings[1], "Changed file text incorrect!");

        testFile.file.replaceWithText (testStrings[2]);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (1500);
        expectEquals (testListener.currentFileText, testStrings[2], "Second changed file text incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("File Change Test");
        fileChangeTest();
    }
};

static FileListenerTest fileListenerTest;
