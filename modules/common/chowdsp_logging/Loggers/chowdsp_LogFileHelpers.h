#pragma once

#include <utility>

namespace chowdsp
{
/** Parameters for log files */
struct LogFileParams
{
    juce::String logFileSubDir {};
    juce::String logFileNameRoot {};
    juce::String logFileExtension = ".log";
    size_t maxNumLogFiles = 50;
    int flushPeriodMilliseconds = 2000;

    static juce::File getLogFile (const LogFileParams&);
};

#ifndef DOXYGEN
namespace LogFileHelpers
{
    constexpr std::string_view openString = "This log file is currently being written to...";
    constexpr std::string_view exitString = "Exiting gracefully...";
    constexpr std::string_view crashString = "Plugin crashing!!!";
    constexpr std::string_view crashExaminedString = "The crash in this log file is now being examined!";

    using FileArray = std::vector<juce::File>;
    FileArray getLogFilesSorted (const LogFileParams& params);

    // delete old log files to keep the total number of log files below the max!
    void pruneOldLogFiles (FileArray& logFiles, const LogFileParams& params);

    void flushLogger (BaseLogger* logger);
    void shutdownLogger (int signal = 0);
} // namespace LogFileHelpers
#endif // DOXYGEN
} // namespace chowdsp
