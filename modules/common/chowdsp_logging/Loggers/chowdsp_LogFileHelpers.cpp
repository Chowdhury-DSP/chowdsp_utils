#include "chowdsp_LogFileHelpers.h"

#if JUCE_MODULE_AVAILABLE_juce_gui_basics
#include <juce_gui_basics/juce_gui_basics.h>
#endif

namespace chowdsp
{
namespace LogFileHelpers
{
    struct LogFileComparator
    {
        static int compareElements (juce::File first, juce::File second) // NOLINT(performance-unnecessary-value-param): JUCE sort needs copies for some reason
        {
            const auto firstTime = first.getLastModificationTime().toMilliseconds();
            const auto secondTime = second.getLastModificationTime().toMilliseconds();

            return firstTime < secondTime;
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
        logFiles.reserve (static_cast<size_t> (numLogFiles));

        for (const auto& entry : juce::RangedDirectoryIterator (logFilesDir, false, logFileWildcard))
        {
            VectorHelpers::insert_sorted (logFiles,
                                          entry.getFile(),
                                          [] (const auto& lhs, const auto& rhs)
                                          {
                                              const auto lhsTime = lhs.getLastModificationTime().toMilliseconds();
                                              const auto rhsTime = rhs.getLastModificationTime().toMilliseconds();
                                              return lhsTime < rhsTime;
                                          });
        }

        return logFiles;
    }

    // delete old log files to keep the total number of log files below the max!
    void pruneOldLogFiles (FileArray& logFiles, const LogFileParams& params)
    {
        if (logFiles.size() <= params.maxNumLogFiles)
            return;

        while (logFiles.size() > params.maxNumLogFiles)
        {
            const auto& lastFile = logFiles.back();
            lastFile.deleteFile();
            logFiles.erase (logFiles.end() - 1);
        }
    }

    void shutdownLogger (int signal)
    {
        juce::Logger::writeToLog (toString (signal == 0 ? exitString : crashString));
        juce::Logger::setCurrentLogger (nullptr);
    }
} // namespace LogFileHelpers
} // namespace chowdsp
