#include "chowdsp_Logger.h"

namespace chowdsp
{
Logger::Logger (const juce::String& logFileSubDir, const juce::String& logFileNameRoot)
    : Logger (LogFileParams { logFileSubDir, logFileNameRoot })
{
}

static juce::File getSystemLogFileFolder()
{
#if JUCE_MAC
    return juce::File ("~/Library/Logs");
#else
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
#endif
}

Logger::Logger (const LogFileParams& loggerParams) : params (loggerParams)
{
    using namespace LogFileHelpers;
    using namespace CrashLogHelpers;

    auto&& pastLogFiles = getLogFilesSorted (params);
    pruneOldLogFiles (pastLogFiles, params);
    checkLogFilesForCrashes (pastLogFiles, crashLogAnalysisCallback);

    log_file = getSystemLogFileFolder()
                   .getChildFile (params.logFileSubDir)
                   .getChildFile (params.logFileNameRoot
                                  + juce::Time::getCurrentTime()
                                        .formatted ("%Y-%m-%d_%H-%M-%S"))
                   .withFileExtension (params.logFileExtension)
                   .getNonexistentSibling();
    log_file.create();

    file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt> (log_file.getFullPathName().toStdString(),
                                                                     false);
    logger.internal_logger.sinks().push_back (file_sink);
    logger.internal_logger.info ("Starting log file: " + log_file.getFullPathName().toStdString());

    juce::Logger::setCurrentLogger (&logger);

    juce::SystemStats::setApplicationCrashHandler (signalHandler);
}

Logger::~Logger()
{
    try
    {
        logger.internal_logger.flush();
    }
    catch ([[maybe_unused]] const spdlog::spdlog_ex& ex)
    {
        jassertfalse;
    }
    LogFileHelpers::shutdownLogger();
}
} // namespace chowdsp
