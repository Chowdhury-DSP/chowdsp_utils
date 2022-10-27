#pragma once

namespace chowdsp
{
/**
 * Utility class for logging diagnostics from a plugin.
 *
 * To use this class, create an instance of it in your plugin class,
 * and provide the subdirectory path to use for storing log files.
 * Log files will be stored in `juce::FileLogger::getSystemLogFileFolder().getChildFile (yourPath)`.
 */
class PluginLogger
{
public:
    struct LoggerParams
    {
        juce::String logFileSubDir;
        juce::String logFileNameRoot;
        juce::String logFileExtension = ".log";
        int maxNumLogFiles = 50;
        std::function<void (const juce::File&)> crashLogAnalysisCallback = [] (const juce::File& logFile)
        { PluginLogger::defaultCrashLogAnalyzer (logFile); };
    };

    PluginLogger (const juce::String& logFileSubDir, const juce::String& logFileNameRoot);
    explicit PluginLogger (LoggerParams loggerParams);
    ~PluginLogger();

    [[nodiscard]] const juce::File& getLogFile() const { return fileLogger->getLogFile(); }
    static void handleCrashWithSignal (int signal);

private:
    static void defaultCrashLogAnalyzer (const juce::File& logFile);

    const LoggerParams params;

    std::unique_ptr<juce::FileLogger> fileLogger;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginLogger)
};
} // namespace chowdsp
