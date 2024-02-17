#pragma once

#include <utility>

namespace chowdsp
{
void defaultCrashLogAnalyzer (const juce::File& logFile);

/** Parameters for log files */
struct LogFileParams
{
    juce::String logFileSubDir {};
    juce::String logFileNameRoot {};
    juce::String logFileExtension = ".log";
    int maxNumLogFiles = 50;
    std::function<void (const juce::File&)> crashLogAnalysisCallback = [] (const juce::File& logFile)
    { defaultCrashLogAnalyzer (logFile); };
};

#ifndef DOXYGEN
namespace LogFileHelpers
{
    constexpr std::string_view openString = "This log file is currently being written to...";
    constexpr std::string_view exitString = "Exiting gracefully...";
    constexpr std::string_view crashString = "Plugin crashing!!!";
    constexpr std::string_view crashExaminedString = "The crash in this log file is now being examined!";

    using FileArray = juce::Array<juce::File>;
    FileArray getLogFilesSorted (const LogFileParams& params);

    // delete old log files to keep the total number of log files below the max!
    void pruneOldLogFiles (FileArray& logFiles, const LogFileParams& params);

    void checkLogFilesForCrashes (const FileArray& logFiles, const LogFileParams& params);

    void shutdownLogger (int signal = 0);
    void signalHandler (void*); // NOSONAR (void* is needed here)
} // namespace LogFileHelpers
#endif // DOXYGEN
} // namespace chowdsp
