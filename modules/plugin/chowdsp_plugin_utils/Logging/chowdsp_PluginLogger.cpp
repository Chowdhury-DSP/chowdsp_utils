#include "chowdsp_PluginLogger.h"

#include <utility>

namespace chowdsp
{
#ifndef DOXYGEN
namespace logger_detail
{
    const juce::String openString = "This log file is currently being written to...";
    const juce::String exitString = "Exiting gracefully...";
    const juce::String crashString = "Plugin crashing!!!";
    const juce::String crashExaminedString = "The crash in this log file is now being examined!";

    struct LogFileComparator
    {
        static int compareElements (juce::File first, juce::File second) // NOLINT(performance-unnecessary-value-param): JUCE sort needs copies for some reason
        {
            const auto firstTime = first.getLastModificationTime().toMilliseconds();
            const auto secondTime = second.getLastModificationTime().toMilliseconds();

            return firstTime < secondTime ? 1 : -1;
        }
    };

    using FileArray = juce::Array<juce::File>;
    static FileArray getLogFilesSorted (const PluginLogger::LoggerParams& params)
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
    static void pruneOldLogFiles (FileArray& logFiles, const PluginLogger::LoggerParams& params)
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

    static void checkLogFilesForCrashes (const FileArray& logFiles, const PluginLogger::LoggerParams& params)
    {
        for (auto& logFile : logFiles)
        {
            const auto& logString = logFile.loadFileAsString();

            if (! logString.contains (crashString))
                continue;

            if (logString.contains (crashExaminedString))
                continue;

            params.crashLogAnalysisCallback (logFile);
            logFile.appendText (crashExaminedString);
        }
    }

    static void shutdownLogger (int signal = 0)
    {
        juce::Logger::writeToLog (signal == 0 ? exitString : crashString);
        juce::Logger::setCurrentLogger (nullptr);
    }

    static void signalHandler (void*) // NOSONAR (void* is needed here)
    {
        juce::Logger::writeToLog ("Interrupt signal received!");
        juce::Logger::writeToLog ("Stack Trace:");
        juce::Logger::writeToLog (juce::SystemStats::getStackBacktrace());

        shutdownLogger (1);
    }
} // namespace logger_detail
#endif // DOXYGEN

PluginLogger::PluginLogger (const juce::String& logFileSubDir, const juce::String& logFileNameRoot) : PluginLogger (LoggerParams { logFileSubDir, logFileNameRoot })
{
}

PluginLogger::PluginLogger (LoggerParams loggerParams) : params (std::move (loggerParams))
{
    using namespace logger_detail;

    auto&& pastLogFiles = getLogFilesSorted (params);
    pruneOldLogFiles (pastLogFiles, params);
    checkLogFilesForCrashes (pastLogFiles, params);

    fileLogger.reset (juce::FileLogger::createDateStampedLogger (
        params.logFileSubDir, params.logFileNameRoot, params.logFileExtension, openString));
    juce::Logger::setCurrentLogger (fileLogger.get());

    juce::SystemStats::setApplicationCrashHandler (signalHandler);
}

PluginLogger::~PluginLogger()
{
    logger_detail::shutdownLogger();
}

void PluginLogger::handleCrashWithSignal (int signal)
{
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wint-to-void-pointer-cast",
                                         "-Wint-to-pointer-cast")
    JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4312)
    logger_detail::signalHandler ((void*) signal);
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    JUCE_END_IGNORE_WARNINGS_MSVC
}

void PluginLogger::defaultCrashLogAnalyzer (const juce::File& logFile)
{
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
}
} // namespace chowdsp
