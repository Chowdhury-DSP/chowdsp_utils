#pragma once

namespace chowdsp::ParamUtils
{
/** Mixin for parameters that recognize some form of modulation. */
struct ModParameterMixin : public clap_juce_extensions::clap_param_extensions
{
    ModParameterMixin() = default;
    virtual ~ModParameterMixin() = default;

    /** Base function for applying monophonic modulation to a parameter. */
    [[maybe_unused]] virtual void applyMonophonicModulation (double /*value*/)
    {
    }

    /** Base function for applying polyphonic modulation to a parameter. */
    [[maybe_unused]] virtual void applyPolyphonicModulation (int32_t /*note_id*/, int16_t /*key*/, int16_t /*channel*/, double /*value*/)
    {
    }
};
} // namespace chowdsp::ParamUtils
