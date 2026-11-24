#include <chowdsp_logging/chowdsp_logging.h>
#include <CatchUtils.h>

namespace
{
// "chowdsp/utils_test_Егор" (written with escapes so the source file encoding doesn't matter)
const juce::String unicodeLogFileSubDir = juce::CharPointer_UTF8 { "chowdsp/utils_test_\xd0\x95\xd0\xb3\xd0\xbe\xd1\x80" };
const juce::String blockedLogFileSubDir = "chowdsp/utils_test_blocked";
const juce::String logFileNameRoot = "chowdsp_utils_test_Log_";
} // namespace

TEST_CASE ("Logger File Path Test", "[common][logs]")
{
    // The loggers share global state with the other logger tests.
    juce::InterProcessLock mutex { "plugin_logger_lock" };
    juce::InterProcessLock::ScopedLockType lock { mutex };
    jassert (lock.isLocked());

    const auto logsRoot = juce::FileLogger::getSystemLogFileFolder();

    SECTION ("Non-ASCII Log File Path")
    {
        // On Windows, if the path is passed to the C runtime as a narrow string,
        // it gets interpreted in the ANSI code page, and the logs end up somewhere else.
        const juce::String testLogString = "This string should be in the log file...";

        juce::File logFile;
        {
            chowdsp::Logger logger { unicodeLogFileSubDir, logFileNameRoot };
            logFile = logger.getLogFile();
            chowdsp::log ("{}", testLogString.toStdString());
        }

        REQUIRE (logFile.getFullPathName().contains (unicodeLogFileSubDir.fromLastOccurrenceOf ("/", false, false)));
        REQUIRE (logFile.existsAsFile());

        const auto logString = logFile.loadFileAsString();
        REQUIRE_MESSAGE (logString.contains (testLogString), "Test log string was not found in the log file!");
        REQUIRE_MESSAGE (logString.contains ("Starting log file: " + logFile.getFullPathName()),
                         "Log file path was not written correctly to the log file!");

        logsRoot.getChildFile (unicodeLogFileSubDir).deleteRecursively();
    }

    SECTION ("Unopenable Log File")
    {
        // Put a regular file where the log directory should be, so the log file can't be created.
        const auto blocker = logsRoot.getChildFile (blockedLogFileSubDir);
        blocker.deleteRecursively();
        REQUIRE (blocker.create().wasOk());
        REQUIRE (blocker.existsAsFile());

        REQUIRE_NOTHROW (
            [&]
            {
                chowdsp::Logger logger { blockedLogFileSubDir + "/logs", logFileNameRoot };
                REQUIRE_FALSE (logger.getLogFile().existsAsFile());

                // logging should still be safe without the file sink
                chowdsp::log ("The magic number is: {}", 18);
                juce::Logger::writeToLog ("Still logging...");
            }());

        blocker.deleteFile();
    }
}
