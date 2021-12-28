#include <test_utils.h>
#include <TimedUnitTest.h>

namespace
{
const StringArray testStrings { "TEST_1", "TEST_2", "TEST_3" };
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
            String currentFileText {};

            explicit TestListener (const File& f) : chowdsp::FileListener (f, 1)
            {
                listenerFileChanged();
            }

            void listenerFileChanged() final
            {
                currentFileText = getListenerFile().loadFileAsString();
            }
        } testListener (testFile.file);

        expectEquals (testListener.currentFileText, testStrings[0], "Initial file text incorrect!");

        MessageManager::getInstance()->runDispatchLoopUntil (1500);

        testFile.file.replaceWithText (testStrings[1]);
        MessageManager::getInstance()->runDispatchLoopUntil (1500);
        expectEquals (testListener.currentFileText, testStrings[1], "Changed file text incorrect!");

        testFile.file.replaceWithText (testStrings[2]);
        MessageManager::getInstance()->runDispatchLoopUntil (1500);
        expectEquals (testListener.currentFileText, testStrings[2], "Second changed file text incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("File Change Test");
        fileChangeTest();
    }
};

static FileListenerTest fileListenerTest;
