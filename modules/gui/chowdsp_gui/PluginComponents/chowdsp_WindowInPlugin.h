#pragma once

namespace chowdsp
{
/**
 * It's not a good idea to create windows from a plugin,
 * so here's a little wrapper class to create a window
 * inside the plugin UI.
 */
template <typename OwnedCompType>
class WindowInPlugin : public juce::DocumentWindow,
                       private juce::ComponentListener
{
public:
    /** Check that the owned component is a juce::Component */
    static_assert (std::is_base_of_v<Component, OwnedCompType>, "Owned Component must be derived from juce::Component");

    /** Create a window for a component with a default constructor */
    explicit WindowInPlugin (Component& creatorComponent) : DocumentWindow (juce::String(), juce::Colours::black, closeButton, false),
                                                            creatorComp (creatorComponent)
    {
        initialise();
    }

    /** Create a window for a component with a non-default constructor */
    template <typename... Args>
    explicit WindowInPlugin (Component& creatorComponent, Args&&... args) : DocumentWindow (juce::String(), juce::Colours::black, closeButton, false),
                                                                            creatorComp (creatorComponent),
                                                                            viewComponent (std::forward<Args> (args)...)
    {
        initialise();
    }

    /** Default destructor */
    ~WindowInPlugin() override
    {
        creatorComp.removeComponentListener (this);
    }

    /** Returns a reference to the component being viewed in the windows */
    OwnedCompType& getViewComponent() { return viewComponent; }

    /** Returns a const reference to the component being viewed in the windows */
    const OwnedCompType& getViewComponent() const { return viewComponent; }

    /** Shows the window as a component within the top-level component */
    void show()
    {
        setBoundsBeforeShowing();
        toFront (true);
        setVisible (true);
    }

    /** Use this method to manually trigger the close button */
    void closeButtonPressed() override { setVisible (false); }

private:
    void initialise()
    {
        creatorComp.addChildComponent (this);
        creatorComp.addComponentListener (this);

        setContentNonOwned (&viewComponent, true);
        setName (viewComponent.getName());
    }

    void componentParentHierarchyChanged (juce::Component&) override
    {
        if (dynamic_cast<juce::AudioProcessorEditor*> (getParentComponent()))
            return; // don't want to go past the plugin editor!

        auto* topLevelComp = creatorComp.getTopLevelComponent();
        jassert (topLevelComp != nullptr);

        if (auto* parent = getParentComponent())
            parent->removeChildComponent (this);

        topLevelComp->addChildComponent (this);
    }

    void setBoundsBeforeShowing()
    {
        auto* parent = getParentComponent();
        jassert (parent != nullptr); // trying to show before adding a parent?? Don't do that.

        auto parentBounds = parent->getLocalBounds();
        setCentrePosition (parentBounds.getCentre());

        if (getPosition().y < 0)
            setTopLeftPosition (getPosition().withY (0));

        if (getPosition().x < 0)
            setTopLeftPosition (getPosition().withX (0));
    }

    juce::Component& creatorComp;
    OwnedCompType viewComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowInPlugin)
};
} // namespace chowdsp
