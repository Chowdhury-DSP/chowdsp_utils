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

Logger::Logger (LogFileParams loggerParams) : params (loggerParams)
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

    file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt> (log_file.getFullPathName().toStdString(),
                                                                     false);
    logger.internal_logger.sinks().push_back (file_sink);

    juce::Logger::setCurrentLogger (&logger);

    juce::SystemStats::setApplicationCrashHandler (signalHandler);
}

Logger::~Logger()
{
    logger.internal_logger.flush();
    LogFileHelpers::shutdownLogger();
}
} // namespace chowdsp
