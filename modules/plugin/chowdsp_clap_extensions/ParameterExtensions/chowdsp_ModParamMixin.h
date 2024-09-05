#pragma once

// LCOV_EXCL_START

namespace chowdsp::ParamUtils
{
/** Mixin for parameters that recognize some form of modulation. */
struct ModParameterMixin : public clap_juce_extensions::clap_juce_parameter_capabilities
{
    ~ModParameterMixin() override = default;

    /** Base function for applying monophonic modulation to a parameter. */
    [[maybe_unused]] void applyMonophonicModulation (double /*value*/) override
    {
    }

    /** Base function for applying polyphonic modulation to a parameter. */
    [[maybe_unused]] void applyPolyphonicModulation (int32_t /*note_id*/, int16_t /*port_index*/, int16_t /*channel*/, int16_t /*key*/, double /*value*/) override
    {
    }
};
} // namespace chowdsp::ParamUtils

// LCOV_EXCL_END
