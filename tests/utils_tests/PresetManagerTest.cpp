#include "DummyPlugin.h"

class PresetManagerTest : public UnitTest
{
public:
    PresetManagerTest() : UnitTest ("Preset Manager Test")
    {
    }

    struct ScopedFile
    {
        ScopedFile (const String& name) : file (File::getSpecialLocation (File::userHomeDirectory).getChildFile (name))
        {
        }

        ~ScopedFile()
        {
            file.deleteRecursively();
        }

        const File file;
    };

    void setParameter (AudioProcessorParameter* param, float value)
    {
        param->setValueNotifyingHost (value);
        MessageManager::getInstance()->runDispatchLoopUntil (200);
    }

    void userPresetTest()
    {
        constexpr float testValue = 0.45f;
        constexpr float dummyValue = 0.75f;

        DummyPlugin plugin;
        auto param = plugin.getParameters()[0];
        setParameter (param, testValue);

        chowdsp::PresetManager presetMgr { plugin.getVTS() };
        ScopedFile presetFile ("test.preset");
        presetMgr.saveUserPreset (presetFile.file);

        setParameter (param, dummyValue);
        expectEquals (param->getValue(), dummyValue, "Changed value is incorrect!");

        presetMgr.loadPresetFromIdx (0);
        expectEquals (param->getValue(), testValue, "Preset value is incorrect!");
    }

    void presetDirtyTest()
    {
        constexpr float initialValue = 0.5f;
        constexpr float testValue1 = 0.25f;
        constexpr float testValue2 = 0.85f;

        DummyPlugin plugin;
        auto param = plugin.getParameters()[0];

        chowdsp::PresetManager presetMgr { plugin.getVTS() };
        expect (! presetMgr.getIsDirty(), "Initial dirty state is incorrect!");

        ScopedFile presetFile1 ("test.preset");
        presetMgr.saveUserPreset (presetFile1.file);
        expect (! presetMgr.getIsDirty(), "Dirty state after saving first preset is incorrect!");

        setParameter (param, testValue1);
        expect (presetMgr.getIsDirty(), "Dirty state after changing value is incorrect!");

        setParameter (param, initialValue);
        expect (presetMgr.getIsDirty(), "Dirty state after return to initial value is incorrect!");

        setParameter (param, testValue2);
        ScopedFile presetFile2 ("test2.preset");
        presetMgr.saveUserPreset (presetFile2.file);
        expect (! presetMgr.getIsDirty(), "Dirty state after saving second preset is incorrect!");

        setParameter (param, testValue2);
        expect (! presetMgr.getIsDirty(), "Dirty state after staying at initial value is incorrect!");

        setParameter (param, testValue1);
        expect (presetMgr.getIsDirty(), "Dirty state after moving from initial value is incorrect!");
    }

    void runTest() override
    {
        beginTest ("User Preset Test");
        userPresetTest();

        beginTest ("Preset Dirty Test");
        presetDirtyTest();
    }
};

static PresetManagerTest presetManagerTest;
