#include "AccessiblePlugin.h"
#include "AccessiblePluginEditor.h"

AccessiblePlugin::AccessiblePlugin() = default;

void AccessiblePlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createPercentParameter (params, "dummy", "Dummy", 0.5f);
}

juce::AudioProcessorEditor* AccessiblePlugin::createEditor()
{
    return new AccessiblePluginEditor (*this);
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AccessiblePlugin();
}
