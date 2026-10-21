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

    /** Logs the current stack trace and flags the log file as a crash. */
    void signalHandler (void*); // NOSONAR (void* is needed here)

    /**
     * Returns true if any of the given stack frames (after our own leading frames)
     * is in the same binary as this module. Returns true if that can't be determined.
     */
    bool framesInvolveThisBinary (const void* const* frames, int numFrames);

    /**
     * Installs the crash handlers, chaining to whatever handlers were
     * already installed (e.g. by the plugin host). Calls are reference counted.
     */
    void installCrashHandler();

    /** Restores the previous handlers once the last install call is balanced. */
    void removeCrashHandler();
} // namespace CrashLogHelpers
#endif
} // namespace chowdsp
