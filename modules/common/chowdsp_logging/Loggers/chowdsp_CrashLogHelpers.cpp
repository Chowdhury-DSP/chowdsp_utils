#include "chowdsp_CrashLogHelpers.h"

namespace chowdsp::CrashLogHelpers
{
// LCOV_EXCL_START
void defaultCrashLogAnalyzer (const juce::File& logFile)
{
#if JUCE_MODULE_AVAILABLE_juce_gui_basics
    auto alertOptions =
        juce::MessageBoxOptions()
            .withTitle ("Crash detected!")
            .withMessage (
                "A previous instance of this plugin has crashed! Would you like to view the logs?")
#if JUCE_IOS
            .withButton ("Copy Logs")
#else
            .withButton ("Show Log File")
#endif
            .withButton ("Cancel");

    juce::AlertWindow::showAsync (
        alertOptions,
        [logFile] (int result)
        {
            if (result == 1)
            {
#if JUCE_IOS
                juce::SystemClipboard::copyTextToClipboard (logFile.loadFileAsString());
#else
                logFile.startAsProcess();
#endif
            }
        });
#else
    jassertfalse; // Implement your own!
#endif
}
// LCOV_EXCL_END

constexpr std::string_view crashString = "Plugin crashing!!!";
constexpr std::string_view crashExaminedString = "The crash in this log file is now being examined!";

void checkLogFilesForCrashes (const LogFileHelpers::FileArray& logFiles,
                              const CrashLogAnalysisCallback& callback)
{
    for (auto& logFile : logFiles)
    {
        const auto& logString = logFile.loadFileAsString();

        if (! logString.contains (toString (crashString)))
            continue;

        if (logString.contains (toString (crashExaminedString)))
            continue;

        callback (logFile);
        logFile.appendText (toString (crashExaminedString));
    }
}

void signalHandler (void*) // NOSONAR (void* is needed here)
{
    juce::Logger::writeToLog ("Interrupt signal received!");
    juce::Logger::writeToLog ("Stack Trace:");
    juce::Logger::writeToLog (juce::SystemStats::getStackBacktrace());

    LogFileHelpers::shutdownLogger (1);
}
} // namespace chowdsp::CrashLogHelpers
