#pragma once

namespace chowdsp
{
/**
 * A struct that will run some code in it's destructor.
 * Inspired by: https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/
 * Inspired by: https://dev.to/pauljlucas/finally-4ke2
 */
template <typename Func>
struct EndOfScopeAction
{
    explicit EndOfScopeAction (Func&& func) : f (std::move (func)), invoke (true) {}

    EndOfScopeAction (const EndOfScopeAction&) = delete;
    EndOfScopeAction& operator= (const EndOfScopeAction&) = delete;

    EndOfScopeAction& operator= (EndOfScopeAction&& other) noexcept
    {
        if (this == &other)
            return *this;

        if (invoke)
            f();

        f = std::move (other.f);
        invoke = std::exchange (other.invoke, false);
        return *this;
    }

    EndOfScopeAction (EndOfScopeAction&& other) noexcept : f (std::move (other.f)),
                                                           invoke (std::exchange (other.invoke, false))
    {
    }

    ~EndOfScopeAction() noexcept
    {
        if (invoke)
            f();
    }

private:
    Func f;
    bool invoke;
};

/** Helper for creating an EndOfScopeAction */
template <typename Func>
[[nodiscard]] EndOfScopeAction<Func> runAtEndOfScope (Func&& f)
{
    return EndOfScopeAction { std::forward<Func> (f) };
}
} // namespace chowdsp
