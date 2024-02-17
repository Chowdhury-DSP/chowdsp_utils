#include "chowdsp_PluginLogger.h"

namespace chowdsp
{
PluginLogger::PluginLogger (const juce::String& logFileSubDir, const juce::String& logFileNameRoot)
    : PluginLogger (LogFileParams { logFileSubDir, logFileNameRoot })
{
}

PluginLogger::PluginLogger (LogFileParams loggerParams) : params (std::move (loggerParams))
{
    using namespace LogFileHelpers;

    auto&& pastLogFiles = getLogFilesSorted (params);
    pruneOldLogFiles (pastLogFiles, params);
    checkLogFilesForCrashes (pastLogFiles, params);

    fileLogger.reset (juce::FileLogger::createDateStampedLogger (params.logFileSubDir,
                                                                    params.logFileNameRoot,
                                                   params.logFileExtension,
                                                      toString (openString)));
    juce::Logger::setCurrentLogger (fileLogger.get());

    juce::SystemStats::setApplicationCrashHandler (signalHandler);
}

PluginLogger::~PluginLogger()
{
    LogFileHelpers::shutdownLogger();
}
} // namespace chowdsp
