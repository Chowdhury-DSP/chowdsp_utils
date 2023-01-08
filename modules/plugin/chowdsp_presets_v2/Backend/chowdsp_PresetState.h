#pragma once

namespace chowdsp
{
/** A chowdsp::StateValueBase for presets */
class PresetState : public StateValueBase
{
    using PresetPtr = OptionalPointer<const Preset>;

public:
    PresetState() : StateValueBase ("chowdsp_preset_manager_preset_state")
    {
    }

    /** Returns the value */
    const Preset* get() const noexcept { return preset.get(); }
    operator const Preset*() const noexcept { return get(); } // NOLINT(google-explicit-constructor): we want to be able to do implicit conversion
    const Preset* operator->() const noexcept { return get(); } // NOLINT(google-explicit-constructor): we want to be able to do implicit conversion

    /** Sets a new value */
    void set (PresetPtr&& v)
    {
        preset = std::move (v);
        changeBroadcaster();
    }

    PresetState& operator= (PresetPtr&& v)
    {
        set (std::move (v));
        return *this;
    }

    PresetState& operator= (const Preset& v)
    {
        PresetPtr presetPtr {};
        presetPtr.setNonOwning (&v);
        set (std::move (presetPtr));
        return *this;
    }

    /** Internal use only! */
    void reset() override
    {
        set ({});
    }

    /** Internal use only! */
    void serialize (JSONSerializer::SerializedType& serial) const override
    {
        JSONSerializer::addChildElement (serial, name);
        if (preset == nullptr)
            JSONSerializer::addChildElement (serial, {});
        else
            JSONSerializer::addChildElement (serial, preset->toJson());
    }

    /** Internal use only! */
    void deserialize (JSONSerializer::DeserializedType deserial) override
    {
        if (deserial.is_null())
        {
            reset();
            return;
        }

        set (PresetPtr { deserial });
    }

private:
    PresetPtr preset {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetState)
};
} // namespace chowdsp
