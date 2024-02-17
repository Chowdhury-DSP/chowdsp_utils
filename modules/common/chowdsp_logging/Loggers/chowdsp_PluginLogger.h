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
    PluginLogger (const juce::String& logFileSubDir, const juce::String& logFileNameRoot);
    explicit PluginLogger (LogFileParams loggerParams);
    ~PluginLogger();

    [[nodiscard]] const juce::File& getLogFile() const { return fileLogger->getLogFile(); }

private:
    const LogFileParams params;

    std::unique_ptr<juce::FileLogger> fileLogger;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginLogger)
};
} // namespace chowdsp
