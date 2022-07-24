#pragma once

namespace chowdsp
{
/** A simple component to display the type, version, and manufacturer of a plugin */
template <typename ProcType = juce::AudioProcessor, typename InfoProvider = StandardInfoProvider>
class InfoComp : public juce::Component
{
public:
    /** Creates an Info component for the given plugin wrapper type */
    explicit InfoComp (const ProcType& processor);

    enum ColourIDs
    {
        text1ColourID, /**< Colour used for plugin type text */
        text2ColourID, /**< Colour used for web link */
    };

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    const ProcType& proc;
    juce::HyperlinkButton linkButton { InfoProvider::getManufacturerString(), InfoProvider::getManufacturerWebsiteURL() };

    int linkX = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoComp)
};
} // namespace chowdsp

#include "chowdsp_InfoComp.cpp"
