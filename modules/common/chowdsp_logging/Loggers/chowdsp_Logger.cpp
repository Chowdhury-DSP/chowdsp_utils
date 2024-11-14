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

Logger::Logger (const LogFileParams& loggerParams,
                CrashLogHelpers::CrashLogAnalysisCallback&& callback)
    : params (loggerParams),
      crashLogAnalysisCallback (std::move (callback))
{
    using namespace LogFileHelpers;
    using namespace CrashLogHelpers;

    auto&& pastLogFiles = getLogFilesSorted (params);
    pruneOldLogFiles (pastLogFiles, params);
    checkLogFilesForCrashes (pastLogFiles, crashLogAnalysisCallback);

    log_file = LogFileParams::getLogFile (params);
    log_file.create();

    file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt> (log_file.getFullPathName().toStdString(),
                                                                     false);
    logger.internal_logger.sinks().push_back (file_sink);
    logger.internal_logger.info ("Starting log file: " + log_file.getFullPathName().toStdString());

    set_global_logger (&logger);

    juce::SystemStats::setApplicationCrashHandler (signalHandler);

    if (loggerParams.flushPeriodMilliseconds > 0)
        startTimer (loggerParams.flushPeriodMilliseconds);
}

Logger::~Logger()
{
    stopTimer();
    LogFileHelpers::shutdownLogger();
}

void Logger::timerCallback()
{
    LogFileHelpers::flushLogger (&logger);
}
} // namespace chowdsp
