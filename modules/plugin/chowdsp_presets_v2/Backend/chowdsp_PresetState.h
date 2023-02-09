#pragma once

namespace chowdsp::presets
{
/** A chowdsp::StateValueBase for presets */
class PresetState : public StateValueBase
{
    using PresetPtr = OptionalPointer<const Preset>;

public:
    PresetState();

    /** Returns the value */
    [[nodiscard]] const Preset* get() const noexcept { return preset.get(); }
    operator const Preset*() const noexcept { return get(); } // NOSONAR NOLINT(google-explicit-constructor): we want to be able to do implicit conversion
    const Preset* operator->() const noexcept { return get(); } // NOLINT(google-explicit-constructor): we want to be able to do implicit conversion

    /** Sets a new value */
    void set (PresetPtr&& v);

    /** Assigns a new value */
    PresetState& operator= (PresetPtr&& v);

    /**
     * Sets a new preset.
     *
     * Note that the user must ensure that the preset is not deleted while
     * it is held in the preset state. If you do need to delete the preset,
     * without changing the preset state, call `assumeOwnership()`.
     */
    PresetState& operator= (const Preset& v);

    /**
     * Copies the preset state and takes ownership of the copy.
     * This can be useful in cases where the preset object needs
     * to be deleted (e.g. re-scanning user presets), without changing
     * the preset state.
     */
    void assumeOwnership();

    /** Internal use only! */
    void reset() override;

    /** Internal use only! */
    void serialize (JSONSerializer::SerializedType& serial) const override;

    /** Internal use only! */
    void deserialize (JSONSerializer::DeserializedType deserial) override;

private:
    PresetPtr preset {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetState)
};

#ifndef DOXYGEN
bool operator== (const PresetState& presetState, std::nullptr_t);
bool operator!= (const PresetState& presetState, std::nullptr_t);
#endif
} // namespace chowdsp::presets
