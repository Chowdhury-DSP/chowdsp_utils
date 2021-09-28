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

        ScopedFile (const File& thisFile) : file (thisFile)
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

        presetMgr.loadPresetFromIndex (0);
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

    void factoryPresetsTest()
    {
        constexpr float testValue = 0.05f;

        DummyPlugin plugin;
        auto param = plugin.getParameters()[0];

        std::vector<chowdsp::Preset> presets;
        {
            chowdsp::PresetManager presetMgr { plugin.getVTS() };

            setParameter (param, testValue);
            ScopedFile presetFile ("test.preset");
            presetMgr.saveUserPreset (presetFile.file);

            presets.emplace_back (presetFile.file);
        }

        chowdsp::PresetManager presetMgr { plugin.getVTS() };
        presetMgr.addPresets (presets);

        presetMgr.loadPresetFromIndex (0);
        expectWithinAbsoluteError (param->getValue(), testValue, 1.0e-6f, "Preset value is incorrect!");
    }

    void processorInterfaceTest()
    {
        DummyPlugin plugin;
        chowdsp::PresetManager presetMgr { plugin.getVTS() };

        ScopedFile presetFile1 ("test1.preset");
        presetMgr.saveUserPreset (presetFile1.file);

        ScopedFile presetFile2 ("test2.preset");
        presetMgr.saveUserPreset (presetFile2.file);

        expectEquals (presetMgr.getNumPresets(), 2, "Num presets incorrect!");

        presetMgr.loadPresetFromIndex (1);
        expectEquals (presetMgr.getCurrentPresetIndex(), 1, "Current preset index incorrect!");

        StringArray presetNames { "test1", "test2" };
        for (int i = 0; i < 2; ++i)
        {
            const auto presetName = presetMgr.getPresetName (i);
            expect (presetNames.contains (presetName), "Preset " + String (i + 1) + " name incorrect!");
            presetNames.removeString (presetName);
        }

        expect (presetNames.isEmpty(), "Some preset names are missing (" + String (presetNames.size()) + ")!");
    }

    void userPresetPathTest()
    {
        DummyPlugin plugin;

        ScopedFile presetPath ("preset_path");
        presetPath.file.createDirectory();

        std::vector<File> presetFiles;
        {
            chowdsp::PresetManager presetMgr { plugin.getVTS() };
            presetMgr.setUserPresetConfigFile ("preset_config.txt");
            presetMgr.setUserPresetPath (presetPath.file);

            auto userPresetPath = presetMgr.getUserPresetPath();

            presetFiles.push_back (userPresetPath.getChildFile ("test1.preset"));
            presetMgr.saveUserPreset (presetFiles.back());

            presetFiles.push_back (userPresetPath.getChildFile ("test2.preset"));
            presetMgr.saveUserPreset (presetFiles.back());

            expectEquals (presetMgr.getNumPresets(), 2, "Initial num presets incorrect!");
        }

        {
            chowdsp::PresetManager presetMgr { plugin.getVTS() };
            presetMgr.setUserPresetConfigFile ("preset_config.txt");

            expectEquals (presetMgr.getNumPresets(), 2, "Num presets after loading incorrect!");

            auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
            expect (userPresetConfigFile.existsAsFile(), "Preset config file does not exist!");
            userPresetConfigFile.deleteRecursively();
        }

        for (auto& file : presetFiles)
            file.deleteRecursively();
    }

    void presetStateTest()
    {
        constexpr float testValue1 = 0.25f;
        constexpr float testValue2 = 0.85f;

        DummyPlugin plugin;
        auto param = plugin.getParameters()[0];

        auto presetMgr = std::make_unique<chowdsp::PresetManager> (plugin.getVTS());

        setParameter (param, testValue1);
        ScopedFile presetFile ("test.preset");
        presetMgr->saveUserPreset (presetFile.file);

        setParameter (param, testValue2);
        expect (presetMgr->getIsDirty(), "Dirty state after changing value is incorrect!");

        auto xml = presetMgr->saveXmlState();
        presetMgr.reset();
        presetMgr = std::make_unique<chowdsp::PresetManager> (plugin.getVTS());
        presetMgr->loadXmlState (xml.get());

        expectEquals (presetMgr->getCurrentPreset()->getName(), String ("test"), "Preset name is incorrect!");
        expect (presetMgr->getIsDirty(), "Dirty state after loading is incorrect!");
    }

    void runTest() override
    {
        beginTest ("User Preset Test");
        userPresetTest();

        beginTest ("Preset Dirty Test");
        presetDirtyTest();

        beginTest ("Factory Presets Test");
        factoryPresetsTest();

        beginTest ("Processor Interface Test");
        processorInterfaceTest();

        beginTest ("User Preset Path Test");
        userPresetPathTest();

        beginTest ("Preset State Test");
        presetStateTest();
    }
};

static PresetManagerTest presetManagerTest;
