#include "../DummyPlugin.h"

/** Empty component filled with a single colour */
class PlainComponent : public Component
{
public:
    PlainComponent (Colour colour)
        : colour (colour)
    {
    }

    void paint (Graphics& g) override
    {
        g.fillAll (colour);
    }

private:
    const Colour colour;
};

/** Test GUI component that includes subcomponents defined
 *  in chowdsp_utils, or modified by chowdsp::ChowLNF:
 *   - Tabbed Component
 *   - Rotary Slider
 *   - Linear Slider
 *   - Toggle Button
 *   - ComboBox
 *   - chowdsp::TitleComp
 *   - chowdsp::InfoComp
 *   - chowdsp::TooltipComp
 *   - chowdsp::PresetsComp
 */
class GUIComponent : public Component
{
public:
    GUIComponent() : tabs (TabbedButtonBar::TabsAtTop)
    {
        LookAndFeel::setDefaultLookAndFeel (lnfAllocator->getLookAndFeel<chowdsp::ChowLNF>());

        slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (Slider::TextBoxBelow, true, 80, 15);
        slider.setName ("Rotary Slider");
        slider.setTooltip ("Testing a rotary slider...");
        addAndMakeVisible (slider);

        linSlider.setSliderStyle (Slider::LinearHorizontal);
        linSlider.setTextBoxStyle (Slider::TextBoxBelow, true, 80, 15);
        linSlider.setName ("Linear Slider");
        linSlider.setTooltip ("Testing a linear slider...");
        linSlider.setColour (Slider::trackColourId, Colours::red);
        addAndMakeVisible (linSlider);

        addAndMakeVisible (toggle);

        auto titleComp = std::make_unique<chowdsp::TitleComp>();
        titleComp->setStrings ("Title", "subtitle", 18.0f);
        tabs.addTab ("Tab1", Colours::darkkhaki, titleComp.release(), true);

        auto infoComp = std::make_unique<chowdsp::InfoComp> (AudioProcessor::WrapperType::wrapperType_AAX);
        tabs.addTab ("Tab2", Colours::darksalmon, infoComp.release(), true);
        addAndMakeVisible (tabs);

        menu.addItemList ({ "Item1", "Item2", "Item3" }, 1);
        menu.setSelectedItemIndex (0, sendNotification);
        addAndMakeVisible (menu);

        addAndMakeVisible (tooltips);

        presetsComp.setNextPrevButton (nullptr, true);
        presetsComp.setNextPrevButton (nullptr, false);
        addAndMakeVisible (presetsComp);

        setSize (500, 500);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::darkgrey);
    }

    void resized() override
    {
        slider.setBounds (0, 0, 100, 100);
        linSlider.setBounds (100, 0, 100, 100);
        toggle.setBounds (200, 0, 100, 100);
        menu.setBounds (300, 40, 100, 30);

        tabs.setBounds (0, 110, getWidth(), 150);
        tooltips.setBounds (0, 275, getWidth(), 25);

        presetsComp.setBounds (300, 85, 200, 35);
    }

    TabbedComponent tabs;

private:
    SharedResourcePointer<chowdsp::LNFAllocator> lnfAllocator;

    Slider slider;
    Slider linSlider;
    ToggleButton toggle { "Toggle" };
    ComboBox menu;

    DummyPlugin dummy { true };
    chowdsp::PresetsComp presetsComp { dummy.getPresetManager() };

    chowdsp::TooltipComponent tooltips;
};

/** Document window to hold the test GUIComponent.
 *  Closes automatically after 10 seconds.
 */
class GUIWindow : public DocumentWindow
{
public:
    GUIWindow() : DocumentWindow ("GUITest", Colours::red, DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar (true);

        comp = std::make_unique<GUIComponent>();
        setContentNonOwned (comp.get(), true);

        centreWithSize (getWidth(), getHeight());

        setVisible (true);

        Timer::callAfterDelay (500, [=] { comp->tabs.setCurrentTabIndex (1); });
        Timer::callAfterDelay (10000, [=] { closeButtonPressed(); });
    }

    void closeButtonPressed() override
    {
        JUCEApplication::getInstance()->systemRequestedQuit();
    }

private:
    std::unique_ptr<GUIComponent> comp;
};

/** Unit test to test components defined in chowdsp_utils
 *  and modified by chowdsp::ChowLNF. This test opens a window
 *  displaying these components, and closes automatically after
 *  10 seconds.
 */
class GUITest : public JUCEApplication
{
public:
    GUITest() {}

    const String getApplicationName() override { return "GUITest"; }
    const String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return false; }

    //==============================================================================
    void initialise (const String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..
        mainWindow = std::make_unique<GUIWindow>();
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..
        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

private:
    std::unique_ptr<GUIWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (GUITest)
