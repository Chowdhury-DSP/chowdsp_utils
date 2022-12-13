#include "chowdsp_PopupMenuHelper.h"

namespace chowdsp
{
PopupMenuHelper::PopupMenuHelper() : PopupMenuHelper (LongPressActionHelper::Parameters())
{
}

PopupMenuHelper::PopupMenuHelper (const LongPressActionHelper::Parameters& params) : longPress (params)
{
    longPress.longPressCallback = [=] (juce::Point<int>)
    { showPopupMenu(); };
}

PopupMenuHelper::~PopupMenuHelper()
{
    setAssociatedComponent (nullptr);
}

void PopupMenuHelper::setAssociatedComponent (juce::Component* comp)
{
    longPress.setAssociatedComponent (comp);

    if (component != nullptr)
        component->removeMouseListener (this);

    component = comp;
    if (component != nullptr)
        component->addMouseListener (this, false);
}

void PopupMenuHelper::showPopupMenu()
{
    juce::PopupMenu menu;
    juce::PopupMenu::Options options;

    if (component != nullptr)
        options = options.withTargetComponent (component);

    options = options.withTargetScreenArea (juce::Rectangle<int> {}.withPosition (juce::Desktop::getMousePosition()));

    popupMenuCallback (menu, options);
    menu.showMenuAsync (options);
}

void PopupMenuHelper::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu())
        showPopupMenu();
}

} // namespace chowdsp
