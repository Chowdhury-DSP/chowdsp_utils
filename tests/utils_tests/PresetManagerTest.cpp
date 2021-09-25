#include "DummyPlugin.h"

class PresetManagerTest : public UnitTest
{
public:
    PresetManagerTest() : UnitTest ("Preset Manager Test")
    {
    }

    void userPresetTest()
    {
        constexpr float testValue = 0.45f;
        constexpr float dummyValue = 0.75f;

        DummyPlugin plugin;
        auto params = plugin.getParameters();
        params[0]->setValueNotifyingHost (testValue);
        MessageManager::getInstance()->runDispatchLoopUntil (200);

        chowdsp::PresetManager presetMgr { &plugin, plugin.getVTS() };
        auto presetFile = File::getSpecialLocation (File::userHomeDirectory).getChildFile ("test.preset");
        presetMgr.saveUserPreset (presetFile);

        params[0]->setValueNotifyingHost (dummyValue);
        MessageManager::getInstance()->runDispatchLoopUntil (200);
        expectEquals (params[0]->getValue(), dummyValue, "Changed value is incorrect!");

        presetMgr.loadPresetFromIdx (0);
        expectEquals (params[0]->getValue(), testValue, "Preset value is incorrect!");

        presetFile.deleteRecursively();
    }

    void runTest() override
    {
        beginTest ("User Preset Test");
        userPresetTest();
    }
};

static PresetManagerTest presetManagerTest;
