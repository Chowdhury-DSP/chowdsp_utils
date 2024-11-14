#include "chowdsp_LogFileHelpers.h"

#if JUCE_MODULE_AVAILABLE_juce_gui_basics
#include <juce_gui_basics/juce_gui_basics.h>
#endif

namespace chowdsp
{
namespace LogFileHelpers
{
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
                                              return lhsTime > rhsTime;
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

    void flushLogger (BaseLogger* logger)
    {
        try
        {
            logger->internal_logger.flush();
        }
        // LCOV_EXCL_START
        catch ([[maybe_unused]] const spdlog::spdlog_ex& ex)
        {
            DBG (std::string { "Unable to flush logger! " } + ex.what());
            jassertfalse;
        }
        // LCOV_EXCL_END
    }

    void shutdownLogger (int signal)
    {
        juce::Logger::writeToLog (toString (signal == 0 ? exitString : crashString));

        if (auto* logger = get_global_logger())
            flushLogger (logger);

        set_global_logger (nullptr);
    }

    static juce::File getSystemLogFileFolder()
    {
#if JUCE_MAC
        return { "~/Library/Logs" };
#else
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
#endif
    }

} // namespace LogFileHelpers

juce::File LogFileParams::getLogFile (const LogFileParams& params)
{
    return LogFileHelpers::getSystemLogFileFolder()
        .getChildFile (params.logFileSubDir)
        .getChildFile (params.logFileNameRoot
                       + juce::Time::getCurrentTime()
                             .formatted ("%Y-%m-%d_%H-%M-%S"))
        .withFileExtension (params.logFileExtension)
        .getNonexistentSibling();
}
} // namespace chowdsp
