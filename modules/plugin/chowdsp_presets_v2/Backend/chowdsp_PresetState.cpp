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

nlohmann::json PresetState::serialize_json() const
{
    if (preset != nullptr)
        return preset->toJson();

    return {};
}

void PresetState::deserialize_json (const nlohmann::json& deserial)
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
