#pragma once

namespace chowdsp
{
struct BaseLogger : juce::Logger
{
    spdlog::logger internal_logger { "chowdsp_log" };
    spdlog::sink_ptr console_sink {};

    BaseLogger()
    {
        console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        internal_logger.sinks().push_back (console_sink);
    }

    void logMessage (const juce::String& message) override
    {
        internal_logger.info (message.toStdString());
    }
};
} // namespace chowdsp
