namespace chowdsp
{
// LCOV_EXCL_START
void HostContextProvider::showParameterContextPopupMenu (const juce::RangedAudioParameter& param,
                                                         const juce::PopupMenu::Options& menuOptions,
                                                         juce::LookAndFeel* lookAndFeel) const
{
    if (const auto contextMenu = getContextMenuForParameter (param))
    {
        auto popupMenu = contextMenu->getEquivalentPopupMenu();
        if (popupMenu.containsAnyActiveItems())
        {
            if (lookAndFeel != nullptr)
                popupMenu.setLookAndFeel (lookAndFeel);
            popupMenu.showMenuAsync (menuOptions.withParentComponent (&editor));
        }
    }
}
// LCOV_EXCL_STOP

std::unique_ptr<juce::HostProvidedContextMenu> HostContextProvider::getContextMenuForParameter (const juce::RangedAudioParameter& param) const
{
    if (const auto* editorContext = editor.getHostContext())
        return editorContext->getContextMenuForParameter (&param);

    return {};
}

void HostContextProvider::registerParameterComponent (juce::Component& comp, const juce::RangedAudioParameter& param)
{
    comp.addComponentListener (this);
    componentToParameterIndexMap.insert_or_assign (&comp, param.getParameterIndex());
}

int HostContextProvider::getParameterIndexForComponent (juce::Component& comp) const
{
    if (const auto iter = componentToParameterIndexMap.find (&comp); iter != componentToParameterIndexMap.end())
        return iter->second;
    return -1;
}

void HostContextProvider::componentBeingDeleted (juce::Component& comp)
{
    componentToParameterIndexMap.erase (&comp);
    comp.removeComponentListener (this);
}
} // namespace chowdsp
