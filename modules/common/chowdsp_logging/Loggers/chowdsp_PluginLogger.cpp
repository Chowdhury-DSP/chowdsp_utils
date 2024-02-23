#include "chowdsp_PluginLogger.h"

namespace chowdsp
{
PluginLogger::PluginLogger (const juce::String& logFileSubDir,
                            const juce::String& logFileNameRoot,
                            CrashLogHelpers::CrashLogAnalysisCallback&& callback)
    : PluginLogger (LogFileParams { logFileSubDir, logFileNameRoot },
                    std::forward<CrashLogHelpers::CrashLogAnalysisCallback> (callback))
{
}

PluginLogger::PluginLogger (LogFileParams loggerParams, CrashLogHelpers::CrashLogAnalysisCallback&& callback)
    : crashLogAnalysisCallback (callback),
      params (std::move (loggerParams))
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
