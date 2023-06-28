#pragma once

namespace chowdsp
{
/**
 * A struct that will run some code in it's destructor.
 * Inspired by: https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/
 */
template <typename Func>
struct EndOfScopeAction
{
    explicit EndOfScopeAction (Func&& func) : f (std::move (func)) {}
    ~EndOfScopeAction() { f(); }

    EndOfScopeAction (const EndOfScopeAction&) = delete;
    EndOfScopeAction& operator= (const EndOfScopeAction&) = delete;

    EndOfScopeAction (EndOfScopeAction&&) noexcept = default;
    EndOfScopeAction& operator= (EndOfScopeAction&&) noexcept = default;

private:
    Func f;
};

/** Helper for creating an EndOfScopeAction */
template <typename Func>
[[nodiscard]] EndOfScopeAction<Func> runAtEndOfScope (Func&& f)
{
    return EndOfScopeAction { std::forward<Func> (f) };
}
} // namespace chowdsp
