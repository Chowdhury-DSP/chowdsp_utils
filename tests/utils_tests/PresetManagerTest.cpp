#include "DummyPlugin.h"
#include "test_utils.h"

using namespace test_utils;

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
        ScopedFile presetPath ("preset_path");
        presetPath.file.createDirectory();
        auto param = plugin.getParameters()[0];
        setParameter (param, testValue);

        chowdsp::PresetManager presetMgr { plugin.getVTS() };
        presetMgr.setUserPresetConfigFile ("preset_config.txt");
        presetMgr.setUserPresetPath (presetPath.file);

        ScopedFile presetFile ("preset_path/test.preset");
        presetMgr.saveUserPreset (presetFile.file);

        setParameter (param, dummyValue);
        expectEquals (param->getValue(), dummyValue, "Changed value is incorrect!");

        presetMgr.loadPresetFromIndex (0);
        expectEquals (param->getValue(), testValue, "Preset value is incorrect!");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
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
        expectWithinAbsoluteError (param->getValue(), testValue, 1.0e-3f, "Preset value is incorrect!");
    }

    void processorInterfaceTest()
    {
        DummyPlugin plugin;
        ScopedFile presetPath ("preset_path");
        presetPath.file.createDirectory();

        chowdsp::PresetManager presetMgr { plugin.getVTS() };
        presetMgr.setUserPresetConfigFile ("preset_config.txt");
        presetMgr.setUserPresetPath (presetPath.file);

        ScopedFile presetFile1 ("preset_path/test1.preset");
        presetMgr.saveUserPreset (presetFile1.file);

        ScopedFile presetFile2 ("preset_path/test2.preset");
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

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
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

        {
            chowdsp::PresetManager presetMgr { plugin.getVTS() };
            presetMgr.setUserPresetConfigFile ("preset_config.txt");
            presetMgr.setUserPresetPath (presetPath.file);
            expectEquals (presetMgr.getNumPresets(), 2, "Num presets after loading incorrect!");

            presetMgr.setUserPresetPath (File());
            expectEquals (presetMgr.getNumPresets(), 2, "Num presets after trying to load empty path incorrect!");

            ScopedFile presetPath2 ("preset_path2");
            presetPath2.file.createDirectory();
            presetMgr.setUserPresetPath (presetPath2.file);
            expectEquals (presetMgr.getNumPresets(), 0, "Num presets after loading empty directory incorrect!");

            auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
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
        ScopedFile presetPath ("preset_path");
        presetPath.file.createDirectory();
        auto param = plugin.getParameters()[0];

        auto presetMgr = std::make_unique<chowdsp::PresetManager> (plugin.getVTS());
        presetMgr->setUserPresetConfigFile ("preset_config.txt");
        presetMgr->setUserPresetPath (presetPath.file);

        setParameter (param, testValue1);
        ScopedFile presetFile ("preset_path/test.preset");
        presetMgr->saveUserPreset (presetFile.file);

        setParameter (param, testValue2);
        expect (presetMgr->getIsDirty(), "Dirty state after changing value is incorrect!");

        auto xml = presetMgr->saveXmlState();
        presetMgr.reset();
        presetMgr = std::make_unique<chowdsp::PresetManager> (plugin.getVTS());
        presetMgr->loadXmlState (xml.get());

        expectEquals (presetMgr->getCurrentPreset()->getName(), String ("test"), "Preset name is incorrect!");
        expect (presetMgr->getIsDirty(), "Dirty state after loading is incorrect!");

        auto userPresetConfigFile = presetMgr->getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
    }

    void defaultPresetTest()
    {
        constexpr float initialValue = 0.5f;
        constexpr float testValue = 0.1f;
        constexpr float testValue2 = 1.0f;

        DummyPlugin plugin;
        ScopedFile presetPath ("preset_path");
        presetPath.file.createDirectory();
        auto param = plugin.getParameters()[0];

        chowdsp::PresetManager presetMgr { plugin.getVTS() };
        presetMgr.setUserPresetConfigFile ("preset_config.txt");
        presetMgr.setUserPresetPath (presetPath.file);

        setParameter (param, testValue);
        ScopedFile presetFile ("preset_path/test.preset");
        presetMgr.saveUserPreset (presetFile.file);
        presetMgr.setDefaultPreset (chowdsp::Preset { presetFile.file });

        setParameter (param, initialValue);
        presetMgr.loadDefaultPreset();
        expectWithinAbsoluteError (param->getValue(), testValue, 1.0e-3f, "Default preset not loaded correctly!");

        presetMgr.setDefaultPreset (chowdsp::Preset { BinaryData::test_preset_preset, BinaryData::test_preset_presetSize });
        presetMgr.loadDefaultPreset();
        expectWithinAbsoluteError (param->getValue(), testValue2, 1.0e-3f, "2nd default preset not loaded correctly!");

        setParameter (param, initialValue);
        presetMgr.loadXmlState (nullptr);
        expectWithinAbsoluteError (param->getValue(), testValue2, 1.0e-3f, "Default preset when trying to load null state not loaded correctly!");

        setParameter (param, initialValue);
        auto badXml = std::make_unique<XmlElement> ("BAD_TAG");
        presetMgr.loadXmlState (badXml.get());
        expectWithinAbsoluteError (param->getValue(), testValue2, 1.0e-3f, "Default preset when trying to load bad state not loaded correctly!");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
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

        beginTest ("Default Preset Test");
        defaultPresetTest();
    }
};

static PresetManagerTest presetManagerTest;
