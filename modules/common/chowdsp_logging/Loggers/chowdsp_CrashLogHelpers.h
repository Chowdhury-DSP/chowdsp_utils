#pragma once

namespace chowdsp
{
#ifndef DOXYGEN
namespace CrashLogHelpers
{
    using CrashLogAnalysisCallback = std::function<void (const juce::File&)>;

    void defaultCrashLogAnalyzer (const juce::File& logFile);

    void checkLogFilesForCrashes (const LogFileHelpers::FileArray& logFiles,
                                  const CrashLogAnalysisCallback& callback);

    void signalHandler (void*); // NOSONAR (void* is needed here)
} // namespace CrashLogHelpers
#endif
} // namespace chowdsp
