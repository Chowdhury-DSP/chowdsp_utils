#include "chowdsp_LogFileHelpers.h"

#if JUCE_MODULE_AVAILABLE_juce_gui_basics
#include <juce_gui_basics/juce_gui_basics.h>
#endif

namespace chowdsp
{
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
#endif
}

namespace LogFileHelpers
{
    struct LogFileComparator
    {
        static int compareElements (juce::File first, juce::File second) // NOLINT(performance-unnecessary-value-param): JUCE sort needs copies for some reason
        {
            const auto firstTime = first.getLastModificationTime().toMilliseconds();
            const auto secondTime = second.getLastModificationTime().toMilliseconds();

            return firstTime < secondTime ? 1 : -1;
        }
    };

    FileArray getLogFilesSorted (const LogFileParams& params)
    {
        const juce::String logFileWildcard = "*" + params.logFileExtension;
        FileArray logFiles;

        auto logFilesDir = juce::FileLogger::getSystemLogFileFolder().getChildFile (params.logFileSubDir);
        if (! logFilesDir.isDirectory())
            return logFiles;

        const auto numLogFiles = logFilesDir.getNumberOfChildFiles (juce::File::findFiles, logFileWildcard);
        logFiles.ensureStorageAllocated (numLogFiles);

        LogFileComparator comparator; // sort (newest files first)
        for (const auto& entry : juce::RangedDirectoryIterator (logFilesDir, false, logFileWildcard))
            logFiles.addSorted (comparator, entry.getFile());

        return logFiles;
    }

    // delete old log files to keep the total number of log files below the max!
    void pruneOldLogFiles (FileArray& logFiles, const LogFileParams& params)
    {
        if (logFiles.size() <= params.maxNumLogFiles)
            return;

        while (logFiles.size() > params.maxNumLogFiles)
        {
            const auto& lastFile = logFiles.getLast();
            lastFile.deleteFile();
            logFiles.removeLast();
        }
    }

    void checkLogFilesForCrashes (const FileArray& logFiles, const LogFileParams& params)
    {
        for (auto& logFile : logFiles)
        {
            const auto& logString = logFile.loadFileAsString();

            if (! logString.contains (toString (crashString)))
                continue;

            if (logString.contains (toString (crashExaminedString)))
                continue;

            params.crashLogAnalysisCallback (logFile);
            logFile.appendText (toString (crashExaminedString));
        }
    }

    void shutdownLogger (int signal)
    {
        juce::Logger::writeToLog (toString (signal == 0 ? exitString : crashString));
        juce::Logger::setCurrentLogger (nullptr);
    }

    void signalHandler (void*) // NOSONAR (void* is needed here)
    {
        juce::Logger::writeToLog ("Interrupt signal received!");
        juce::Logger::writeToLog ("Stack Trace:");
        juce::Logger::writeToLog (juce::SystemStats::getStackBacktrace());

        shutdownLogger (1);
    }
} // namespace LogFileHelpers
} // namespace chowdsp
