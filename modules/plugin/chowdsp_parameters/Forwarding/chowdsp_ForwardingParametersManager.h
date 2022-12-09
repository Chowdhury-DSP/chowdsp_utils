#pragma once

namespace chowdsp
{
/** Parameter information needed for forwarding */
struct ParameterForwardingInfo
{
    juce::RangedAudioParameter* param = nullptr;
    juce::String name {};
};

/**
 * Useful class for managing groups of ForwardingParameter
 *
 * @tparam Provider                     Template provider type, which must provide static
 *                                      `juce::String getForwardingParameterID()` method.
 * @tparam totalNumForwardingParameters The total number of parameters to be created and managed by this class.
 */
template <typename Provider, int totalNumForwardingParameters>
class ForwardingParametersManager
{
public:
    /** Initializes handles to the forwarding parameters, and connects them to the given processor */
    explicit ForwardingParametersManager (juce::AudioProcessorValueTreeState& vts) : processor (vts.processor)
    {
        for (int i = 0; i < totalNumForwardingParameters; ++i)
        {
            auto id = Provider::getForwardingParameterID (i);
            auto forwardedParam = std::make_unique<ForwardingParameter> (id, nullptr, "Blank");

            forwardedParam->setProcessor (&vts.processor);
            forwardedParams[(size_t) i] = forwardedParam.get();
            vts.processor.addParameter (forwardedParam.release());
        }
    }

    ~ForwardingParametersManager()
    {
        clearParameterRange (0, (int) forwardedParams.size());
    }

    /** Returns a flat array of the forwarded parameters */
    [[maybe_unused]] auto& getForwardedParameters() { return forwardedParams; }

    /** Returns a flat array of the forwarded parameters */
    [[maybe_unused]] const auto& getForwardedParameters() const { return forwardedParams; }

    /**
     * Sets a range of parameters.
     *
     * @param startIndex            The start of the range to set.
     * @param endIndex              The end (exclusive) of the range to set.
     * @param paramInfoProvider     A lambda which accepts the parameter index, and returns the corresponding ParameterForwardingInfo.
     * @param deferHostNotification If this is true, `updateHostDisplay()` will be deferred until all the parameter have been updated.
     */
    template <typename ParamInfoProvider>
    void setParameterRange (int startIndex,
                            int endIndex,
                            const ParamInfoProvider& paramInfoProvider,
                            bool deferHostNotification = true)
    {
        for (int i = startIndex; i < endIndex; ++i)
        {
            auto [param, paramName] = paramInfoProvider (i);
            forwardedParams[(size_t) i]->setParam (param, paramName, deferHostNotification);
        }

        if (deferHostNotification)
            ForwardingParameter::reportParameterInfoChange (&processor);
    }

    /**
     * Clears a range of parameters to nullptr.
     *
     * @param startIndex            The start of the range to set.
     * @param endIndex              The end (exclusive) of the range to set.
     * @param deferHostNotification If this is true, `updateHostDisplay()` will be deferred until all the parameter have been updated.
     */
    void clearParameterRange (int startIndex, int endIndex, bool deferHostNotification = true)
    {
        setParameterRange (
            startIndex,
            endIndex,
            [] (int) -> ParameterForwardingInfo
            { return {}; },
            deferHostNotification);
    }

protected:
    std::array<ForwardingParameter*, (size_t) totalNumForwardingParameters> forwardedParams;

private:
    juce::AudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ForwardingParametersManager)
};
} // namespace chowdsp
