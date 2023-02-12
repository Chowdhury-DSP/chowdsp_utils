#include "chowdsp_PresetState.h"

namespace chowdsp::presets
{
PresetState::PresetState() : StateValueBase ("chowdsp_preset_manager_preset_state")
{
}

void PresetState::set (PresetPtr&& v)
{
    preset = std::move (v);
    changeBroadcaster();
}

PresetState& PresetState::operator= (PresetPtr&& v)
{
    set (std::move (v));
    return *this;
}

PresetState& PresetState::operator= (const Preset& v)
{
    set (PresetPtr { &v, false });
    return *this;
}

void PresetState::assumeOwnership()
{
    if (preset.isOwner())
        return;

    preset = PresetPtr (*preset);
}

void PresetState::reset()
{
    set ({});
}

void PresetState::serialize (JSONSerializer::SerializedType& serial) const
{
    JSONSerializer::addChildElement (serial, name);
    if (preset == nullptr)
        JSONSerializer::addChildElement (serial, {});
    else
        JSONSerializer::addChildElement (serial, preset->toJson());
}

void PresetState::deserialize (JSONSerializer::DeserializedType deserial)
{
    if (deserial.is_null())
    {
        reset();
        return;
    }

    set (PresetPtr { deserial });
}

bool operator== (const PresetState& presetState, std::nullptr_t)
{
    return presetState.get() == nullptr;
}

bool operator!= (const PresetState& presetState, std::nullptr_t)
{
    return ! (presetState == nullptr);
}
} // namespace chowdsp::presets
