#include <chowdsp_logging/chowdsp_logging.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <CatchUtils.h>
#include <future>

namespace
{
const juce::String logFileSubDir = "chowdsp/utils_test";
const juce::String logFileNameRoot = "chowdsp_utils_test_Log_";
} // namespace

TEMPLATE_TEST_CASE ("Plugin Logger Test", "[common][logs]", chowdsp::PluginLogger, chowdsp::Logger)
{
    juce::InterProcessLock mutex { "plugin_logger_lock" };
    juce::GenericScopedLock lock { mutex };

    using Logger = TestType;

    SECTION ("Basic Log Test")
    {
        const juce::String testLogString = "This string should be in the log file...";
        const juce::String testNonLogString = "This string should not be in the log file...";

        juce::File logFile;
        {
            Logger logger { logFileSubDir, logFileNameRoot };
            juce::Logger::writeToLog (testLogString);

            logFile = logger.getLogFile();
        }

        juce::Logger::writeToLog (testNonLogString);

        auto logString = logFile.loadFileAsString();
        REQUIRE_MESSAGE (logString.contains (testLogString), "Test log string was not found in the log file!");
        REQUIRE_MESSAGE (! logString.contains (testNonLogString), "Test non-log string WAS found in the log file!");
    }

    SECTION ("Limit Num Log Files To Test")
    {
        constexpr int numLoggersAtOnce = 5;
        constexpr int numIters = 20;

        auto logsDirectory = juce::FileLogger::getSystemLogFileFolder().getChildFile (logFileSubDir);
        auto getNumLogFiles = [&]
        {
            return logsDirectory.getNumberOfChildFiles (juce::File::findFiles, "*");
        };

        for (int i = 0; i < numIters; ++i)
        {
            using LoggerPtr = std::unique_ptr<Logger>;
            std::vector<std::future<LoggerPtr>> futures;
            futures.reserve (numLoggersAtOnce);
            for (int j = 0; j < numLoggersAtOnce; ++j)
                futures.push_back (std::async (std::launch::async, []
                                               { return std::make_unique<Logger> (logFileSubDir, logFileNameRoot); }));

            auto numLogFiles = getNumLogFiles();
            REQUIRE_MESSAGE (numLogFiles <= 55, "Too many log files found in logs directory!");
        }

        logsDirectory.deleteRecursively();
    }

#if ! JUCE_LINUX
    SECTION ("Crash Log Test")
    {
        juce::File logFile;
        {
            Logger logger { logFileSubDir, logFileNameRoot };
            logFile = logger.getLogFile();
            int signal = 44;
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wint-to-void-pointer-cast")
            chowdsp::CrashLogHelpers::signalHandler ((void*) signal);
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        }

        auto logString = logFile.loadFileAsString();
        REQUIRE_MESSAGE (logString.contains ("Interrupt signal received!"), "Interrupt signal string not found in log!");
        REQUIRE_MESSAGE (logString.contains ("Stack Trace:"), "Stack trace string not found in log!");
        REQUIRE_MESSAGE (logString.contains ("Plugin crashing!!!"), "Plugin crashing string not found in log!");
        REQUIRE_MESSAGE (! logString.contains ("Exiting gracefully"), "Exit string string WAS found in the log file!");

        // the first logger we create after a crash should report the crash...
        {
            auto prevNumTopLevelWindows = juce::TopLevelWindow::getNumTopLevelWindows();
            Logger logger { logFileSubDir, logFileNameRoot };
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            auto newNumTopLevelWindows = juce::TopLevelWindow::getNumTopLevelWindows();
            REQUIRE_MESSAGE (newNumTopLevelWindows == prevNumTopLevelWindows + 1, "AlertWindow not created!");

            // Linux on CI doesn't like trying to open the log file!
            for (int i = 0; i < newNumTopLevelWindows; ++i)
            {
                if (auto* alertWindow = dynamic_cast<juce::AlertWindow*> (juce::TopLevelWindow::getTopLevelWindow (i)))
                {
                    alertWindow->triggerButtonClick ("Show Log File");
                    juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
                    break;
                }
            }
        }

        // the next logger should not!
        {
            auto prevNumTopLevelWindows = juce::TopLevelWindow::getNumTopLevelWindows();
            Logger logger { logFileSubDir, logFileNameRoot };

            auto newNumTopLevelWindows = juce::TopLevelWindow::getNumTopLevelWindows();
            REQUIRE_MESSAGE (newNumTopLevelWindows == prevNumTopLevelWindows, "AlertWindow was incorrectly created!");
        }
    }
#endif

    // clean up after ourselves...
    auto logsDirectory = juce::FileLogger::getSystemLogFileFolder().getChildFile (logFileSubDir);
    logsDirectory.deleteRecursively();
}
