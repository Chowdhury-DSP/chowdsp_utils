#pragma once

/** Custom formatter for juce::String */
template <>
struct fmt::formatter<juce::String> : formatter<std::string>
{
    static auto format (const juce::String& str, format_context& ctx) -> decltype (ctx.out())
    {
        return fmt::format_to (ctx.out(), "{}", str.toStdString());
    }
};

/** Custom formatter for std::span */
template <typename T>
struct fmt::formatter<nonstd::span<T>> : formatter<std::string>
{
    static auto format (nonstd::span<const T> span, format_context& ctx) -> decltype (ctx.out())
    {
        return fmt::format_to (ctx.out(), "{{{}}}", fmt::join (span, ","));
    }
};

namespace chowdsp
{
/** Implementation of fmt::vformat using a memory arena. */
template <typename ArenaType>
std::string_view vformat (ArenaType& arena, fmt::string_view format_str, fmt::format_args args)
{
    using FormatAllocator = STLArenaAllocator<char, ArenaType>;
    FormatAllocator alloc { arena };
    fmt::basic_memory_buffer<char, 1, FormatAllocator> buffer { alloc };
    fmt::vformat_to (std::back_inserter (buffer), format_str, args);
    return { buffer.data(), buffer.size() };
}

/** Implementation of fmt::format using a memory arena. */
template <typename ArenaType, typename... Args>
std::string_view format (ArenaType& arena, fmt::string_view format_str, const Args&... args)
{
    return vformat (arena, format_str, fmt::make_format_args (args...));
}

/** Implementation of fmt::format using the global logger's memory arena. */
template <typename... Args>
std::string_view format (fmt::string_view format_str, const Args&... args)
{
    auto* global_logger = get_global_logger();
    if (global_logger == nullptr)
    {
        // make sure you've set up the global logger before calling this!
        jassertfalse;
        return {};
    }

    // If this is being called from multiple threads, we might need to synchronize the arena here?
    return format (global_logger->arena, format_str, std::forward<const Args&> (args)...);
}

/** A formatted wrapped for juce::Logger::writeToLog(). */
template <typename... Args>
void log (fmt::string_view format_str, const Args&... args)
{
    juce::Logger::writeToLog (toString (format (format_str, std::forward<const Args&> (args)...)));
}
} // namespace chowdsp
