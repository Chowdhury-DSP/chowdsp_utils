#pragma once

template <>
struct fmt::formatter<juce::String> : formatter<std::string>
{
  static auto format (const juce::String& str, format_context& ctx) -> decltype (ctx.out())
  {
    return fmt::format_to (ctx.out(), "{}", str.toStdString());
  }
};

template <typename T>
struct fmt::formatter<nonstd::span<T>> : formatter<std::string>
{
  static auto format (nonstd::span<const T> span, format_context& ctx) -> decltype (ctx.out())
  {
    return fmt::format_to (ctx.out(), "{}", fmt::join (span, ","));
  }
};

namespace chowdsp
{
template <typename ArenaType>
std::string_view vformat (ArenaType& arena, fmt::string_view format_str, fmt::format_args args)
{
  using FormatAllocator = STLArenaAllocator<char, ArenaType>;
  FormatAllocator alloc { arena };
  fmt::basic_memory_buffer<char, 0, FormatAllocator> buffer { alloc };
  fmt::vformat_to (std::back_inserter (buffer), format_str, args);
  return { buffer.data(), buffer.size() };
}

template <typename ArenaType, typename... Args>
std::string_view format (ArenaType& arena, fmt::string_view format_str, const Args&... args)
{
  return vformat (arena, format_str, fmt::make_format_args (args...));
}

template <typename... Args>
std::string_view format (fmt::string_view format_str, const Args&... args)
{
  // If this is being called from multiple threads, we might need to synchronize the arena here?
  return format (get_global_logger()->arena, format_str, std::forward<const Args&> (args)...);
}

template <typename... Args>
juce::String jformat (fmt::string_view format_str, const Args&... args)
{
  return toString (format (format_str, std::forward<const Args&> (args)...));
}
template <typename... Args>
void log (fmt::string_view format_str, const Args&... args)
{
  juce::Logger::writeToLog (jformat (format_str, std::forward<const Args&> (args)...));
}
}
