namespace chowdsp
{
#ifndef DOXYGEN
namespace parameters_view_detail
{
    //==============================================================================
    class BooleanParameterComponent : public juce::Component
    {
    public:
        BooleanParameterComponent (BoolParameter& param, ParameterListeners& listeners)
            : attachment (param, listeners, button, nullptr)
        {
            setComponentID (param.paramID);
            addAndMakeVisible (button);
        }

        void resized() override
        {
            auto area = getLocalBounds();
            area.removeFromLeft (8);
            button.setBounds (area.reduced (0, 10));
        }

    private:
        juce::ToggleButton button;
        ButtonAttachment attachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BooleanParameterComponent)
    };

    class ChoiceParameterComponent : public juce::Component
    {
    public:
        ChoiceParameterComponent (ChoiceParameter& param, ParameterListeners& listeners)
            : attachment (param, listeners, box, nullptr)
        {
            setComponentID (param.paramID);
            addAndMakeVisible (box);
        }

        void resized() override
        {
            auto area = getLocalBounds();
            area.removeFromLeft (8);
            box.setBounds (area.reduced (0, 10));
        }

    private:
        juce::ComboBox box;
        ComboBoxAttachment attachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChoiceParameterComponent)
    };

    class SliderParameterComponent : public juce::Component
    {
    public:
        SliderParameterComponent (FloatParameter& param, ParameterListeners& listeners)
            : attachment (param, listeners, slider, nullptr)
        {
            setComponentID (param.paramID);
            slider.setScrollWheelEnabled (false);
            addAndMakeVisible (slider);
        }

        void resized() override
        {
            auto area = getLocalBounds().reduced (3, 10);
            slider.setBounds (area);
        }

    private:
        juce::Slider slider { juce::Slider::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxRight };
        SliderAttachment attachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderParameterComponent)
    };

    //==============================================================================
    class ParameterDisplayComponent : public juce::Component
    {
    public:
        ParameterDisplayComponent (juce::RangedAudioParameter& param, ParameterListeners& listeners)
            : parameter (param)
        {
            parameterName.setText (parameter.getName (128), juce::dontSendNotification);
            parameterName.setJustificationType (juce::Justification::centredRight);
            parameterName.setInterceptsMouseClicks (false, false);
            addAndMakeVisible (parameterName);

            parameterLabel.setText (parameter.getLabel(), juce::dontSendNotification);
            parameterLabel.setInterceptsMouseClicks (false, false);
            addAndMakeVisible (parameterLabel);

            addAndMakeVisible (*(parameterComp = createParameterComp (listeners)));
            setComponentID (parameterComp->getComponentID());

            setSize (400, 40);
        }

        void resized() override
        {
            auto area = getLocalBounds();

            parameterName.setBounds (area.removeFromLeft (100));
            parameterLabel.setBounds (area.removeFromRight (50));
            parameterComp->setBounds (area);
        }

    private:
        juce::RangedAudioParameter& parameter;
        juce::Label parameterName, parameterLabel;
        std::unique_ptr<juce::Component> parameterComp;

        std::unique_ptr<juce::Component> createParameterComp (ParameterListeners& listeners) const
        {
            if (auto* boolParam = dynamic_cast<BoolParameter*> (&parameter))
                return std::make_unique<BooleanParameterComponent> (*boolParam, listeners);

            if (auto* choiceParam = dynamic_cast<ChoiceParameter*> (&parameter))
                return std::make_unique<ChoiceParameterComponent> (*choiceParam, listeners);

            if (auto* sliderParam = dynamic_cast<FloatParameter*> (&parameter))
                return std::make_unique<SliderParameterComponent> (*sliderParam, listeners);

            return {};
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterDisplayComponent)
    };

    //==============================================================================
    struct ParamControlItem : public juce::TreeViewItem
    {
        ParamControlItem (juce::RangedAudioParameter& paramIn, ParameterListeners& paramListeners)
            : param (paramIn), listeners (paramListeners) {}

        bool mightContainSubItems() override { return false; }

        std::unique_ptr<juce::Component> createItemComponent() override
        {
            return std::make_unique<ParameterDisplayComponent> (param, listeners);
        }

        [[nodiscard]] int getItemHeight() const override { return 40; }

        juce::RangedAudioParameter& param;
        ParameterListeners& listeners;
    };

    struct ParameterGroupItem : public juce::TreeViewItem
    {
        ParameterGroupItem (ParamHolder& params, ParameterListeners& listeners)
            : name (params.getName())
        {
            params.doForAllParameterContainers (
                [this, &listeners] (auto& paramVec)
                {
                    for (auto& param : paramVec)
                        addSubItem (std::make_unique<ParamControlItem> (param, listeners).release());
                },
                [this, &listeners] (auto& paramHolder)
                {
                    addSubItem (std::make_unique<ParameterGroupItem> (paramHolder, listeners).release());
                });
        }

        bool mightContainSubItems() override { return getNumSubItems() > 0; }

        std::unique_ptr<juce::Component> createItemComponent() override
        {
            return std::make_unique<juce::Label> (name, name);
        }

        juce::String name;
    };
} // namespace parameters_view_detail
#endif

//==============================================================================
struct ParametersView::Pimpl
{
    Pimpl (ParamHolder& params, ParameterListeners& listeners)
        : groupItem (params, listeners)
    {
        const auto numIndents = getNumIndents (groupItem);
        const auto width = 400 + view.getIndentSize() * numIndents;

        view.setSize (width, 400);
        view.setDefaultOpenness (true);
        view.setRootItemVisible (false);
        view.setRootItem (&groupItem);
    }

    static int getNumIndents (const juce::TreeViewItem& item)
    {
        int maxInner = 0;

        for (auto i = 0; i < item.getNumSubItems(); ++i)
            maxInner = juce::jmax (maxInner, 1 + getNumIndents (*item.getSubItem (i)));

        return maxInner;
    }

    [[nodiscard]] juce::Component* getComponentForParameter (const juce::RangedAudioParameter& param) const
    {
        return getComponentForParameter (param, *view.getRootItem(), view);
    }

    static juce::Component* getComponentForParameter (const juce::RangedAudioParameter& param,
                                                      const juce::TreeViewItem& item,
                                                      const juce::TreeView& tree)
    {
        for (int i = 0; i < item.getNumSubItems(); ++i)
        {
            if (const auto* subItem = item.getSubItem (i))
            {
                if (auto* paramControlItem = dynamic_cast<const parameters_view_detail::ParamControlItem*> (subItem))
                {
                    if (&paramControlItem->param == &param)
                        return tree.getItemComponent (subItem);
                }

                if (auto* comp = getComponentForParameter (param, *subItem, tree))
                    return comp;
            }
        }
        return nullptr;
    }

    parameters_view_detail::ParameterGroupItem groupItem;
    juce::TreeView view;
};

//==============================================================================
ParametersView::ParametersView (PluginState& pluginState, ParamHolder& params)
    : ParametersView (pluginState.getParameterListeners(), params)
{
}

ParametersView::ParametersView (ParameterListeners& listeners, ParamHolder& params)
    : pimpl (std::make_unique<Pimpl> (params, listeners))
{
    auto* viewport = pimpl->view.getViewport();

    setOpaque (true);
    addAndMakeVisible (pimpl->view);

    setSize (viewport->getViewedComponent()->getWidth() + viewport->getVerticalScrollBar().getWidth(),
             juce::jlimit (125, 400, viewport->getViewedComponent()->getHeight()));
}

ParametersView::~ParametersView() = default;

void ParametersView::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void ParametersView::resized()
{
    pimpl->view.setBounds (getLocalBounds());
}

juce::Component* ParametersView::getComponentForParameter (const juce::RangedAudioParameter& param)
{
    return pimpl->getComponentForParameter (param);
}
} // namespace chowdsp
