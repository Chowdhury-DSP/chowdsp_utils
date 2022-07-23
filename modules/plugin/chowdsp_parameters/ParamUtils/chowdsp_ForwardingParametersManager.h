#pragma once

namespace chowdsp
{
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
    explicit ForwardingParametersManager (juce::AudioProcessorValueTreeState& vts)
    {
        for (int i = 0; i < totalNumForwardingParameters; ++i)
        {
            auto id = Provider::getForwardingParameterID (i);
            auto forwardedParam = std::make_unique<chowdsp::ForwardingParameter> (id, nullptr, "Blank");

            forwardedParam->setProcessor (&vts.processor);
            forwardedParams[(size_t) i] = forwardedParam.get();
            vts.processor.addParameter (forwardedParam.release());
        }
    }

    ~ForwardingParametersManager()
    {
        for (auto* param : forwardedParams)
            param->setParam (nullptr);
    }

    /** Returns a flat array of the forwarded parameters */
    [[maybe_unused]] auto& getForwardedParameters() { return forwardedParams; }

    /** Returns a flat array of the forwarded parameters */
    [[maybe_unused]] const auto& getForwardedParameters() const { return forwardedParams; }

protected:
    std::array<ForwardingParameter*, (size_t) totalNumForwardingParameters> forwardedParams;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ForwardingParametersManager)
};
} // namespace chowdsp
