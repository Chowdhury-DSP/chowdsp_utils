#include "chowdsp_Logger.h"

namespace chowdsp
{
Logger::Logger (const juce::String& logFileSubDir,
                const juce::String& logFileNameRoot,
                CrashLogHelpers::CrashLogAnalysisCallback&& callback)
    : Logger (LogFileParams { logFileSubDir, logFileNameRoot },
              std::move (callback))
{
}

static void handleOldLogFiles (LogFileParams params,
                               CrashLogHelpers::CrashLogAnalysisCallback callback)
{
    using namespace LogFileHelpers;
    using namespace CrashLogHelpers;
    if (! params.cleanupOnBackgroundThread)
    {
        auto&& pastLogFiles = getLogFilesSorted (params);
        pruneOldLogFiles (pastLogFiles, params);
        checkLogFilesForCrashes (pastLogFiles, callback);
        return;
    }

    params.cleanupOnBackgroundThread = false;
    juce::Thread::launch (
        [params, callback]
        {
            handleOldLogFiles (params, callback);
        });
}

Logger::Logger (const LogFileParams& loggerParams,
                CrashLogHelpers::CrashLogAnalysisCallback&& callback)
    : params (loggerParams),
      crashLogAnalysisCallback (std::move (callback))
{
    using namespace LogFileHelpers;
    using namespace CrashLogHelpers;

    handleOldLogFiles (params, crashLogAnalysisCallback);

    log_file = LogFileParams::getLogFile (params);
    log_file.create();

    try
    {
#if JUCE_WINDOWS
        const spdlog::filename_t log_file_path { log_file.getFullPathName().toWideCharPointer() };
#else
        const spdlog::filename_t log_file_path { log_file.getFullPathName().toStdString() };
#endif
        file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt> (log_file_path, false);
        logger.internal_logger.sinks().push_back (file_sink);
        logger.internal_logger.info ("Starting log file: " + log_file.getFullPathName().toStdString());
    }
    catch (const spdlog::spdlog_ex& e)
    {
        // Something went wrong when trying to open the log file...
        // Falling back to the logger's default sink(s).
        file_sink.reset();
        logger.internal_logger.error ("Unable to open log file {}: {}", log_file.getFullPathName().toStdString(), e.what());
    }

    set_global_logger (&logger);

    installCrashHandler();

    if (loggerParams.flushPeriodMilliseconds > 0)
        startTimer (loggerParams.flushPeriodMilliseconds);
}

Logger::~Logger()
{
    stopTimer();
    LogFileHelpers::shutdownLogger();
    CrashLogHelpers::removeCrashHandler();
}

void Logger::timerCallback()
{
    LogFileHelpers::flushLogger (&logger);
}
} // namespace chowdsp
