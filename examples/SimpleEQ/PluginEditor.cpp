#include "PluginEditor.h"
#include <chowdsp_serialization/chowdsp_serialization.h>

using Serializer = chowdsp::JSONSerializer;

namespace
{
const juce::String eqParamsExt = "*.eqparams";
const auto defaultSaveLocation = juce::File::getSpecialLocation (juce::File::userDesktopDirectory);
} // namespace

PluginEditor::PluginEditor (SimpleEQPlugin& p) : juce::AudioProcessorEditor (p),
                                                 plugin (p),
                                                 genericEditor (plugin),
                                                 plots (plugin.getState(), plugin.getState().params.eqParams)
{
    addAndMakeVisible (genericEditor);
    addAndMakeVisible (saveButton);
    addAndMakeVisible (loadButton);
    addAndMakeVisible (plots);

    setSize (500, 700);

    saveButton.onClick = [this]
    {
        constexpr auto fileBrowserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;
        fileChooser = std::make_shared<juce::FileChooser> ("Save EQ Params", defaultSaveLocation, eqParamsExt, true, false, getTopLevelComponent());
        fileChooser->launchAsync (fileBrowserFlags,
                                  [&] (const juce::FileChooser& fc)
                                  {
                                      if (fc.getResults().isEmpty())
                                          return;

                                      const auto result = fc.getResult();
                                      result.create();

                                      const auto params = plugin.getEQParams();
                                      chowdsp::Serialization::serialize<Serializer> (params, result);
                                  });
    };

    loadButton.onClick = [this]
    {
        constexpr auto fileBrowserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        fileChooser = std::make_shared<juce::FileChooser> ("Load EQ Params", defaultSaveLocation, eqParamsExt, true, false, getTopLevelComponent());
        fileChooser->launchAsync (fileBrowserFlags,
                                  [&] (const juce::FileChooser& fc)
                                  {
                                      if (fc.getResults().isEmpty())
                                          return;

                                      const auto result = fc.getResult();
                                      result.create();

                                      PrototypeEQ::Params params {};
                                      chowdsp::Serialization::deserialize<Serializer> (result, params);
                                      plugin.loadEQParams (params);
                                  });
    };
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void PluginEditor::resized()
{
    auto bounds = getLocalBounds();

    plots.setBounds (bounds.removeFromTop (200));
    genericEditor.setBounds (bounds.removeFromTop (bounds.proportionOfHeight (0.93f)));

    const auto buttonWidth = proportionOfWidth (0.33f);
    saveButton.setBounds (bounds.removeFromLeft (buttonWidth));
    loadButton.setBounds (bounds.removeFromLeft (buttonWidth));
}
