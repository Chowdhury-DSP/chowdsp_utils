#include "chowdsp_UIState.h"

namespace
{
const juce::Identifier lastSizeTag { "ui_state-last-size" };
const juce::Identifier widthTag { "ui_state-ui-width" };
const juce::Identifier heightTag { "ui_state-ui-height" };
} // namespace

namespace chowdsp
{
UIState::UIState (juce::AudioProcessorValueTreeState& vtState, int defWidth, int defHeight) : vts (vtState),
                                                                                              defaultWidth (defWidth),
                                                                                              defaultHeight (defHeight)
{
}

void UIState::attachToComponent (juce::AudioProcessorEditor& comp)
{
    comp.addComponentListener (this);
    comp.setBounds (getLastEditorSize());
}

juce::Rectangle<int> UIState::getLastEditorSize() const
{
    auto sizeNode = vts.state.getOrCreateChildWithName (lastSizeTag, nullptr);
    if (! sizeNode.hasProperty (widthTag) || ! sizeNode.hasProperty (heightTag))
        return { defaultWidth, defaultHeight };

    return { sizeNode.getProperty (widthTag), sizeNode.getProperty (heightTag) };
}

void UIState::componentBeingDeleted (juce::Component& component)
{
    component.removeComponentListener (this);
}

void UIState::componentMovedOrResized (juce::Component& component, bool /*wasMoved*/, bool wasResized)
{
    if (wasResized)
        setLastEditorSize (component.getBounds());
}

void UIState::setLastEditorSize (const juce::Rectangle<int>& bounds)
{
    auto sizeNode = vts.state.getOrCreateChildWithName (lastSizeTag, nullptr);
    sizeNode.setProperty (widthTag, bounds.getWidth(), nullptr);
    sizeNode.setProperty (heightTag, bounds.getHeight(), nullptr);
}
} // namespace chowdsp
