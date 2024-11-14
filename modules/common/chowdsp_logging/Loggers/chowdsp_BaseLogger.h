#pragma once

namespace chowdsp
{
struct BaseLogger : juce::Logger
{
    spdlog::logger internal_logger { "chowdsp_log" };
    spdlog::sink_ptr console_sink {};
    Broadcaster<void (const juce::String&)> onLogMessage {};
    ChainedArenaAllocator arena { 8192 };
    static BaseLogger* global_logger;

    BaseLogger()
    {
        console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        internal_logger.sinks().push_back (console_sink);

#if JUCE_DEBUG && JUCE_WINDOWS
        internal_logger.sinks().push_back (std::make_shared<spdlog::sinks::msvc_sink_mt>());
#endif
    }

    void logMessage (const juce::String& message) override
    {
        internal_logger.info (message.toStdString());
        onLogMessage (message);
        arena.clear();
    }
};

/** Set's a (static) logger to be used globally. */
void set_global_logger (BaseLogger*);

/** Returns the global logger, or nullptr if none has been set. */
BaseLogger* get_global_logger();
} // namespace chowdsp
