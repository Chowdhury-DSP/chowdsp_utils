#pragma once

namespace chowdsp
{
class Logger
{
public:
    Logger (const juce::String& logFileSubDir,
            const juce::String& logFileNameRoot,
            CrashLogHelpers::CrashLogAnalysisCallback&& callback = &CrashLogHelpers::defaultCrashLogAnalyzer);
    explicit Logger (const LogFileParams& loggerParams,
                     CrashLogHelpers::CrashLogAnalysisCallback&& callback = &CrashLogHelpers::defaultCrashLogAnalyzer);
    ~Logger();

    [[nodiscard]] const juce::File& getLogFile() const { return log_file; }

    const LogFileParams params;

private:
    CrashLogHelpers::CrashLogAnalysisCallback crashLogAnalysisCallback = &CrashLogHelpers::defaultCrashLogAnalyzer;

    juce::File log_file {};
    spdlog::sink_ptr file_sink {};

    BaseLogger logger;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Logger)
};
} // namespace chowdsp
