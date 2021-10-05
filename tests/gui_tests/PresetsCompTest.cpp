#include "../DummyPlugin.h"
#include "../test_utils.h"

using namespace test_utils;

class PresetsCompTest : public UnitTest
{
public:
    PresetsCompTest() : UnitTest ("Presets Component Test")
    {
    }

    void presetBoxTextTest()
    {
        DummyPlugin plugin { true };
        auto& presetMgr = plugin.getPresetManager();
        auto param = plugin.getParameters()[0];

        ScopedFile presetFile1 ("Test1.preset");
        presetMgr.saveUserPreset (presetFile1.file);

        setParameter (param, 0.1f);
        ScopedFile presetFile2 ("Test2.preset");
        presetMgr.saveUserPreset (presetFile2.file);

        chowdsp::PresetsComp presetsComp { presetMgr };
        expectEquals (presetsComp.getPresetMenuText(), String ("Test2"), "Initial preset text is incorrect!");

        setParameter (param, 0.9f);
        expectEquals (presetsComp.getPresetMenuText(), String ("Test2*"), "Dirty preset text is incorrect!");

        auto newPresetIndex = 1 - presetMgr.getCurrentPresetIndex();
        presetMgr.loadPresetFromIndex (newPresetIndex);
        expectEquals (presetsComp.getPresetMenuText(), String ("Test1"), "Loaded preset text is incorrect!");

        setParameter (param, 0.9f);
        expectEquals (presetsComp.getPresetMenuText(), String ("Test1*"), "Dirty preset text is incorrect!");
    }

    void runTest() override
    {
        beginTest ("Preset Box Text Test");
        presetBoxTextTest();
    }
};

static PresetsCompTest presetsCompTest;
