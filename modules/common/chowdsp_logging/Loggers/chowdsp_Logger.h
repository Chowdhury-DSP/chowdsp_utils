#pragma once

namespace chowdsp
{
class Logger
{
public:
    Logger (const juce::String& logFileSubDir, const juce::String& logFileNameRoot);
    explicit Logger (const LogFileParams& loggerParams);
    ~Logger();

    [[nodiscard]] const juce::File& getLogFile() const { return log_file; }

    LogFileParams params;
    CrashLogHelpers::CrashLogAnalysisCallback crashLogAnalysisCallback = &CrashLogHelpers::defaultCrashLogAnalyzer;

private:
    juce::File log_file {};
    spdlog::sink_ptr file_sink {};

    BaseLogger logger;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Logger)
};
} // namespace chowdsp
