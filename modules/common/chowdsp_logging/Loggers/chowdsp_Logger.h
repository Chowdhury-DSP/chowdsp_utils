#pragma once

namespace chowdsp
{
class Logger
{
public:
    Logger (const juce::String& logFileSubDir, const juce::String& logFileNameRoot);
    explicit Logger (LogFileParams loggerParams);
    ~Logger();

    // [[nodiscard]] const juce::File& getLogFile() const { return fileLogger->getLogFile(); }

    LogFileParams params;

private:
    spdlog::sink_ptr file_sink {};

    BaseLogger internal_logger;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Logger)
};
} // namespace chowdsp
