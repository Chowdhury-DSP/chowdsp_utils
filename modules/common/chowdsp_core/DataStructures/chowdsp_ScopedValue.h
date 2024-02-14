#pragma once

namespace chowdsp
{
/**
 * Scoped object that stores a value from a variable on construction
 * and writes the final value back to that variable on deletion.
 *
 * For tight inner loops in DSP code, accessing member variables
 * isn't always ideal, since the compiler can't optimize as much
 * as it can for locally stored values. In those situations, construct
 * a ScopedValue with the member variable before the loop, and the final
 * value will be stored back in the member when it goes out of scope.
 */
template <typename T>
class ScopedValue
{
public:
    explicit ScopedValue (T& val) : value (val), ref (val) {}

    ScopedValue (const ScopedValue&) = delete;
    ScopedValue& operator= (const ScopedValue&) = delete;

    ScopedValue (ScopedValue&&) = default;
    ScopedValue& operator= (ScopedValue&&) = default;

    ~ScopedValue() { ref = value; }

    /** Returns a reference to the value */
    inline T& get() { return value; }

private:
    T value;
    T& ref;
};
} // namespace chowdsp
