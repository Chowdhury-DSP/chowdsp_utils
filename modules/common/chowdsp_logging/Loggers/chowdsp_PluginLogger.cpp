#include "chowdsp_PluginLogger.h"

namespace chowdsp
{
PluginLogger::PluginLogger (const juce::String& logFileSubDir,
                            const juce::String& logFileNameRoot,
                            CrashLogHelpers::CrashLogAnalysisCallback&& callback)
    : PluginLogger (LogFileParams { logFileSubDir, logFileNameRoot },
                    std::move (callback))
{
}

PluginLogger::PluginLogger (LogFileParams loggerParams, CrashLogHelpers::CrashLogAnalysisCallback&& callback)
    : params (std::move (loggerParams)),
      crashLogAnalysisCallback (std::move (callback))
{
    using namespace LogFileHelpers;
    using namespace CrashLogHelpers;

    auto&& pastLogFiles = getLogFilesSorted (params);
    pruneOldLogFiles (pastLogFiles, params);
    checkLogFilesForCrashes (pastLogFiles, crashLogAnalysisCallback);

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
