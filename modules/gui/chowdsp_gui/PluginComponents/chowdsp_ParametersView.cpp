namespace chowdsp
{
#ifndef DOXYGEN
namespace parameters_view_detail
{
    //==============================================================================
    class BooleanParameterComponent : public juce::Component
    {
    public:
        BooleanParameterComponent (BoolParameter& param, PluginState& pluginState)
            : attachment (param, pluginState, button)
        {
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
        ChoiceParameterComponent (ChoiceParameter& param, PluginState& pluginState)
            : attachment (param, pluginState, box)
        {
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
        SliderParameterComponent (FloatParameter& param, PluginState& pluginState)
            : attachment (param, pluginState, slider)
        {
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
        ParameterDisplayComponent (juce::Component& editorIn, juce::RangedAudioParameter& param, PluginState& pluginState)
            : editor (editorIn), parameter (param)
        {
            parameterName.setText (parameter.getName (128), juce::dontSendNotification);
            parameterName.setJustificationType (juce::Justification::centredRight);
            parameterName.setInterceptsMouseClicks (false, false);
            addAndMakeVisible (parameterName);

            parameterLabel.setText (parameter.getLabel(), juce::dontSendNotification);
            parameterLabel.setInterceptsMouseClicks (false, false);
            addAndMakeVisible (parameterLabel);

            addAndMakeVisible (*(parameterComp = createParameterComp (pluginState)));

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
        juce::Component& editor;
        juce::RangedAudioParameter& parameter;
        juce::Label parameterName, parameterLabel;
        std::unique_ptr<juce::Component> parameterComp;

        std::unique_ptr<juce::Component> createParameterComp (PluginState& pluginState) const
        {
            if (auto* boolParam = dynamic_cast<BoolParameter*> (&parameter))
                return std::make_unique<BooleanParameterComponent> (*boolParam, pluginState);

            if (auto* choiceParam = dynamic_cast<ChoiceParameter*> (&parameter))
                return std::make_unique<ChoiceParameterComponent> (*choiceParam, pluginState);

            if (auto* sliderParam = dynamic_cast<FloatParameter*> (&parameter))
                return std::make_unique<SliderParameterComponent> (*sliderParam, pluginState);

            return {};
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterDisplayComponent)
    };

    //==============================================================================
    struct ParamControlItem : public juce::TreeViewItem
    {
        ParamControlItem (juce::Component& editorIn, juce::RangedAudioParameter& paramIn, PluginState& pluginState)
            : editor (editorIn), param (paramIn), state (pluginState) {}

        bool mightContainSubItems() override { return false; }

        std::unique_ptr<juce::Component> createItemComponent() override
        {
            return std::make_unique<ParameterDisplayComponent> (editor, param, state);
        }

        [[nodiscard]] int getItemHeight() const override { return 40; }

        juce::Component& editor;
        juce::RangedAudioParameter& param;
        PluginState& state;
    };

    struct ParameterGroupItem : public juce::TreeViewItem
    {
        ParameterGroupItem (juce::Component& editor, ParamHolder& params, PluginState& pluginState)
            : name (params.getName())
        {
            params.template doForAllParameterContainers (
                [&] (auto& paramVec)
                {
                    for (auto& param : paramVec)
                    {
                        addSubItem (std::make_unique<ParamControlItem> (editor,
                                                                        param,
                                                                        pluginState)
                                        .release());
                    }
                },
                [&] (auto& paramHolder)
                {
                    addSubItem (std::make_unique<ParameterGroupItem> (editor,
                                                                      paramHolder,
                                                                      pluginState)
                                    .release());
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
    Pimpl (juce::Component& editor, ParamHolder& params, PluginState& pluginState)
        : groupItem (editor, params, pluginState)
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

    parameters_view_detail::ParameterGroupItem groupItem;
    juce::TreeView view;
};

//==============================================================================
ParametersView::ParametersView (PluginState& pluginState, ParamHolder& params)
    : pimpl (std::make_unique<Pimpl> (*this, params, pluginState))
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
} // namespace chowdsp
