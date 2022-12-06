#include "TimedUnitTest.h"
#include <DummyPlugin.h>
#include "test_utils.h"
#include <chowdsp_gui/chowdsp_gui.h>
#include <chowdsp_parameters/chowdsp_parameters.h>

class HostContextProviderTest : public TimedUnitTest
{
public:
    HostContextProviderTest() : TimedUnitTest ("Host Context Provider Test") {}

    void getContextMenuForParameterTest()
    {
        juce::AudioProcessor::setTypeOfNextNewPlugin (juce::AudioProcessor::wrapperType_VST3);
        DummyPlugin plugin { true };
        std::unique_ptr<juce::AudioProcessorEditor> editor (plugin.createEditor());
        chowdsp::HostContextProvider hostContextProvider (plugin, *editor);

        auto&& menu = hostContextProvider.getContextMenuForParameter (*plugin.getVTS().getParameter ("dummy"));
        expect (menu == nullptr); // since we're not in a host, we won't get a context menu back :(

        juce::AudioProcessor::setTypeOfNextNewPlugin (juce::AudioProcessor::wrapperType_Undefined);
    }

    void getParameterIndexForComponentTest()
    {
        DummyPlugin plugin { true };

        struct Editor : juce::AudioProcessorEditor
        {
            explicit Editor (DummyPlugin& plugin) : juce::AudioProcessorEditor (plugin),
                                                    hcp (plugin, *this)
            {
                setSize (100, 100);
            }

            void paint (juce::Graphics& g) override
            {
                g.fillAll (juce::Colours::black);
            }

            chowdsp::HostContextProvider hcp;

            std::unique_ptr<juce::Component> paramComponent;
        } editor { plugin };

        editor.paramComponent = std::make_unique<juce::Component>();
        editor.hcp.registerParameterComponent (*editor.paramComponent, *plugin.getVTS().getParameter ("dummy"));
        expectEquals (editor.hcp.getParameterIndexForComponent (*editor.paramComponent), 0, "Incorrect param index for component!");
        editor.paramComponent.reset();

        editor.paramComponent = std::make_unique<juce::Component>();
        expectEquals (editor.hcp.getParameterIndexForComponent (*editor.paramComponent), -1, "Incorrect param index for unregistered component!");
    }

    void runTestTimed() override
    {
        beginTest ("Get Context Menu For Parameter Test");
        getContextMenuForParameterTest();

        beginTest ("Get Parameter Index For Component Test");
        getParameterIndexForComponentTest();
    }
};
static HostContextProviderTest hostContextProviderTest;
