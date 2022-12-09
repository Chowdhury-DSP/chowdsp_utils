#pragma once

namespace chowdsp
{
/** Type to use for parameters that can be used to initialise a ValueTreeState */
using Parameters = std::vector<std::unique_ptr<juce::RangedAudioParameter>>;

namespace ParamUtils
{
    using VTSParam = juce::AudioProcessorValueTreeState::Parameter;

    /** Returns a parameter of a given type from the AudioProcessorValueTreeState */
    template <typename ParameterPointerType>
    ParameterPointerType getParameterPointer (const juce::AudioProcessorValueTreeState& vts, const ParameterID& parameterID)
    {
        static_assert (std::is_pointer_v<ParameterPointerType>, "Parameter pointer type must be a pointer!");
        static_assert (std::is_base_of_v<juce::RangedAudioParameter, std::remove_pointer_t<ParameterPointerType>>, "Parameter type must be derived from juce::RangedAudioParameter");

#if JUCE_VERSION < 0x070000
        auto* baseParameter = vts.getParameter (parameterID);
#else
        auto* baseParameter = vts.getParameter (parameterID.getParamID());
#endif
        jassert (baseParameter != nullptr); // parameter was not found in the ValueTreeState!

        auto* typedParameter = dynamic_cast<ParameterPointerType> (baseParameter);
        jassert (typedParameter != nullptr); // parameter has the incorrect type!

        return typedParameter;
    }

    /** Loads a parameter of a given type from the AudioProcessorValueTreeState */
    template <typename ParameterPointerType>
    void loadParameterPointer (ParameterPointerType& parameter, const juce::AudioProcessorValueTreeState& vts, const ParameterID& parameterID)
    {
        parameter = getParameterPointer<ParameterPointerType> (vts, parameterID);
    }

    /**
     * Useful alias for `params.push_back (std::make_unique<ParamType> (args...));`
     */
    template <typename ParamType, typename... Args>
    void emplace_param (Parameters& params, Args&&... args)
    {
        params.push_back (std::make_unique<ParamType> (std::forward<Args> (args)...));
    }

    /** Useful method for creating a juce::NormalisableRange with a centre value */
    template <typename T>
    juce::NormalisableRange<T> createNormalisableRange (T start, T end, T centre)
    {
        auto range = juce::NormalisableRange { start, end };
        range.setSkewForCentre (centre);

        return range;
    }

    /** Helper method for creating frequency parameters */
    void createFreqParameter (Parameters& params, const ParameterID& id, const juce::String& name, float min, float max, float centre, float defaultValue);

    /** Helper method for creating percent parameters */
    void createPercentParameter (Parameters& params, const ParameterID& id, const juce::String& name, float defaultValue);

    /** Helper method for creating bipolar percent parameters */
    void createBipolarPercentParameter (Parameters& params, const ParameterID& id, const juce::String& name, float defaultValue = 0.0f);

    /** Helper method for creating gain parameters in Decibels */
    void createGainDBParameter (Parameters& params, const ParameterID& id, const juce::String& name, float min, float max, float defaultValue, float centerValue = -1000.0f);

    /** Helper method for creating time parameters in milliseconds */
    void createTimeMsParameter (Parameters& params, const ParameterID& id, const juce::String& name, const juce::NormalisableRange<float>& range, float defaultValue);

    /** Helper method for creating ratio parameters */
    void createRatioParameter (Parameters& params, const ParameterID& id, const juce::String& name, const juce::NormalisableRange<float>& range, float defaultValue = 1.0f);
} // namespace ParamUtils
} // namespace chowdsp
