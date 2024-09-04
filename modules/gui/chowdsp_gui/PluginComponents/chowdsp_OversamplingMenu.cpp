#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>
#include "chowdsp_OversamplingMenu.h"

// LCOV_EXCL_START

namespace chowdsp
{
template <typename OSType, typename ComboBoxType>
void OversamplingMenu<OSType, ComboBoxType>::initialise (juce::AudioProcessorValueTreeState& vts)
{
    this->setDescription ("Oversampling Menu");

    this->setColour (backgroundColourID, juce::Colours::transparentBlack);
    this->setColour (textColourID, juce::Colours::white);
    this->setColour (outlineColourID, juce::Colours::white);
    this->setColour (accentColourID, juce::Colours::yellow);

    std::tie (parameters[OSParam],
              parameters[OSMode],
              parameters[OSOfflineParam],
              parameters[OSOfflineMode],
              parameters[OSOfflineSame]) = osManager.getParameters();

    // at least these parameters much be initialized
    jassert (parameters[OSParam] != nullptr);
    jassert (parameters[OSMode] != nullptr);

    offlineParamsAvailable = parameters[OSOfflineParam] != nullptr;

    int count = 0;
    for (auto* param : parameters)
    {
        attachments[count].reset();
        if (param == nullptr)
            continue;

        attachments[count] = std::make_unique<juce::ParameterAttachment> (
            *param,
            [this] (float)
            { generateComboBoxMenu(); },
            vts.undoManager);

        count += 1;
    }

    sampleRateOrBlockSizeChangedCallback = osManager.sampleRateOrBlockSizeChangedBroadcaster.connect (
        [thisComp = this]
        {
            juce::MessageManager::callAsync (
                [safeThis = juce::Component::SafePointer { thisComp }]
                {
                    if (safeThis != nullptr)
                        safeThis->generateComboBoxMenu();
                });
        });
}

template <typename OSType, typename ComboBoxType>
void OversamplingMenu<OSType, ComboBoxType>::updateColours()
{
    this->setColour (juce::ComboBox::ColourIds::backgroundColourId, this->findColour (backgroundColourID));
    this->setColour (juce::ComboBox::ColourIds::textColourId, this->findColour (textColourID));
    this->setColour (juce::ComboBox::ColourIds::outlineColourId, this->findColour (outlineColourID));

    auto newAccentColour = this->findColour (accentColourID);
    if (accentColour != newAccentColour)
    {
        accentColour = newAccentColour;
        generateComboBoxMenu();
    }

    this->repaint();
}

template <typename OSType, typename ComboBoxType>
void OversamplingMenu<OSType, ComboBoxType>::generateComboBoxMenu()
{
    this->clear();
    if (! osManager.hasBeenPrepared())
        return;

    auto* menu = this->getRootMenu();

    auto createParamItem = [this] (juce::PopupMenu::Item& item,
                                   auto* parameter,
                                   auto& attachment,
                                   int& menuIdx,
                                   int menuOffset,
                                   const juce::String& choice,
                                   bool forceOff = false,
                                   bool disableSame = false)
    {
        item.itemID = menuIdx++;
        int paramVal = item.itemID - menuOffset;
        bool isSelected = ((int) parameter->convertFrom0to1 (parameter->getValue()) == paramVal) && ! forceOff;
        item.text = choice;
        item.colour = isSelected ? accentColour : juce::Colours::white;
        item.action = [this, paramVal, disableSame, &attachment]
        {
            if (disableSame)
                attachments[OSOfflineSame]->setValueAsCompleteGesture (0.0f);
            attachment->setValueAsCompleteGesture (float (paramVal));

            generateComboBoxMenu();
        };
        return isSelected;
    };

    // set up main menu
    juce::StringArray headers { "OS Factor", "Mode", "OS Factor", "Mode" };
    int menuIdx = 1;
    int menuOffset = menuIdx;

    // set up offline menu
    juce::PopupMenu offlineMenu;
    int offlineMenuIdx = 1;
    int offlineMenuOffset = menuIdx;

    bool sameAsRT = false;
    if (offlineParamsAvailable) // same as real-time option
    {
        juce::PopupMenu::Item item;
        item.itemID = menuIdx++;
        auto* parameter = parameters[OSOfflineSame];
        sameAsRT = parameter != nullptr && (int) parameter->convertFrom0to1 (parameter->getValue()) == 1;
        item.text = "Same as real-time";
        item.colour = sameAsRT ? accentColour : juce::Colours::white;
        item.action = [this]
        {
            attachments[OSOfflineSame]->setValueAsCompleteGesture (1.0f);
            generateComboBoxMenu();
        };
        offlineMenu.addItem (item);
    }

    // add parameter to menus
    juce::String osParamText, osOfflineParamText;
    for (int paramIdx = 0; paramIdx < 4; ++paramIdx)
    {
        if (parameters[paramIdx] == nullptr)
            continue;

        bool isOfflineParam = paramIdx >= OSOfflineParam;
        auto* thisMenu = isOfflineParam ? &offlineMenu : menu;
        auto& thisMenuIdx = isOfflineParam ? offlineMenuIdx : menuIdx;
        auto& thisMenuOffset = isOfflineParam ? offlineMenuOffset : menuOffset;
        thisMenuOffset = thisMenuIdx;

        thisMenu->addSectionHeader (headers[paramIdx]);
        for (auto& choice : parameters[paramIdx]->getAllValueStrings())
        {
            juce::PopupMenu::Item item;
            bool isSelected = createParamItem (item,
                                               parameters[paramIdx],
                                               attachments[paramIdx],
                                               thisMenuIdx,
                                               thisMenuOffset,
                                               choice,
                                               sameAsRT && isOfflineParam,
                                               isOfflineParam);
            thisMenu->addItem (item);

            if (isSelected && paramIdx == OSParam)
                osParamText = item.text;
            else if (isSelected && paramIdx == OSOfflineParam)
                osOfflineParamText = item.text;
        }
    }

    juce::String comboBoxText = osParamText;
    if (! sameAsRT && offlineParamsAvailable)
        comboBoxText += " / " + osOfflineParamText;
    this->setText (comboBoxText);

    if (offlineParamsAvailable)
    {
        menu->addSeparator();
        menu->addSubMenu ("Offline:", offlineMenu);
    }

    auto osParam = parameters[0] != nullptr ? parameters[0]->convertFrom0to1 (parameters[0]->getValue()) : 0;
    auto osMode = parameters[1] != nullptr ? parameters[1]->convertFrom0to1 (parameters[1]->getValue()) : 0;
    auto osIndex = osManager.getOSIndex ((int) osParam, (int) osMode);
    auto curLatencyMs = osManager.getLatencyMilliseconds (osIndex);
    menu->addSectionHeader ("Oversampling Latency: " + juce::String (curLatencyMs, 3) + " ms");
}

template class OversamplingMenu<VariableOversampling<float>>;
template class OversamplingMenu<VariableOversampling<double>>;
} // namespace chowdsp

// LCOV_EXCL_END
