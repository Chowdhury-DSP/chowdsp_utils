#include "TimedUnitTest.h"
#include <DummyPlugin.h>
#include "test_utils.h"
#include <chowdsp_gui/chowdsp_gui.h>
#include <chowdsp_parameters/chowdsp_parameters.h>

class HostContextProviderTest : public TimedUnitTest
{
public:
    HostContextProviderTest() : TimedUnitTest ("Host Context Provider Test") {}


    void showParameterContextPopupMenuTest ()
    {
        using namespace chowdsp::ParamUtils;

        DummyPlugin plugin{ true };
        chowdsp::HostContextProvider<DummyPlugin> hostContextProvider(plugin, *plugin.createEditor());

        chowdsp::Parameters params;
        plugin.addParameters(params);
    }

    void getContextMenuForParameterTest()
    {


    }

    void registerParameterComponentTest()
    {   

    }

    void getParameterIndexForComponentTest()
    {

    }

    void componentBeingDeletedTest()
    {

    }

    void runTestTimed() override
    {
        beginTest ("Show Parameter Context Popup Menu Test");
        showParameterContextPopupMenuTest();

        beginTest("Get Context Menu For Parameter Test");
        getContextMenuForParameterTest();

        beginTest("Register Parameter Component Test");
        registerParameterComponentTest();

        beginTest("Get Parameter Index For Component Test");
        getParameterIndexForComponentTest();

        beginTest("Component Being Deleted Test");
        componentBeingDeletedTest();



    }
};
static HostContextProviderTest hostContextProviderTest;
