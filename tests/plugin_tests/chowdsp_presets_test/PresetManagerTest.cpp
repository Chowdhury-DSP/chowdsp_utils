#include <chowdsp_presets/chowdsp_presets.h>
#include <DummyPlugin.h>
#include <test_utils.h>
#include <TimedUnitTest.h>
#include "TestPresetBinaryData.h"

using namespace test_utils;

class PresetManagerTest : public TimedUnitTest
{
public:
    PresetManagerTest() : TimedUnitTest ("Preset Manager Test", "Presets")
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

        presetMgr.loadPreset (presetMgr.getPresetMap().begin()->second);
        expectEquals (param->getValue(), testValue, "Preset value is incorrect!");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteFile();
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

        juce::StringArray presetNames { "test1", "test2" };
        for (int i = 0; i < 2; ++i)
        {
            const auto presetName = presetMgr.getPresetName (i);
            expect (presetNames.contains (presetName), "Preset " + juce::String (i + 1) + " name incorrect!");
            presetNames.removeString (presetName);
        }

        expect (presetNames.isEmpty(), "Some preset names are missing (" + juce::String (presetNames.size()) + ")!");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteFile();
    }

    void userPresetPathTest()
    {
        DummyPlugin plugin;

        ScopedFile presetPath ("preset_path");
        presetPath.file.createDirectory();

        std::vector<juce::File> presetFiles;
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
            userPresetConfigFile.deleteFile();
        }

        {
            chowdsp::PresetManager presetMgr { plugin.getVTS() };
            presetMgr.setUserPresetConfigFile ("preset_config.txt");
            presetMgr.setUserPresetPath (presetPath.file);
            expectEquals (presetMgr.getNumPresets(), 2, "Num presets after loading incorrect!");

            presetMgr.setUserPresetPath (juce::File());
            expectEquals (presetMgr.getNumPresets(), 2, "Num presets after trying to load empty path incorrect!");

            ScopedFile presetPath2 ("preset_path2");
            presetPath2.file.createDirectory();
            presetMgr.setUserPresetPath (presetPath2.file);
            expectEquals (presetMgr.getNumPresets(), 0, "Num presets after loading empty directory incorrect!");

            auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
            userPresetConfigFile.deleteFile();
        }

        for (auto& file : presetFiles)
            file.deleteFile();
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

        expectEquals (presetMgr->getCurrentPreset()->getName(), juce::String ("test"), "Preset name is incorrect!");
        expect (presetMgr->getIsDirty(), "Dirty state after loading is incorrect!");

        auto userPresetConfigFile = presetMgr->getUserPresetConfigFile();
        userPresetConfigFile.deleteFile();
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
        expect (chowdsp::Preset { presetFile.file } == *presetMgr.getDefaultPreset(), "Default preset not set correctly!");

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
        auto badXml = std::make_unique<juce::XmlElement> ("BAD_TAG");
        presetMgr.loadXmlState (badXml.get());
        expectWithinAbsoluteError (param->getValue(), testValue2, 1.0e-3f, "Default preset when trying to load bad state not loaded correctly!");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteFile();
    }

    void triggerPresetListUpdateTest()
    {
        struct TestListener : chowdsp::PresetManager::Listener
        {
            bool listenerCalled = false;

            void presetListUpdated() override
            {
                listenerCalled = true;
            }
        } testListener;

        DummyPlugin plugin;
        chowdsp::PresetManager presetMgr { plugin.getVTS() };
        presetMgr.addListener (&testListener);

        presetMgr.triggerPresetListUpdate();
        expect (testListener.listenerCalled, "Preset list update not triggered!");
    }

    void getUserPresetsTest()
    {
        DummyPlugin plugin;

        ScopedFile presetPath ("preset_path");
        presetPath.file.createDirectory();
        std::vector<juce::File> presetFiles;

        chowdsp::PresetManager presetMgr { plugin.getVTS() };
        presetMgr.setUserPresetConfigFile ("preset_config.txt");
        presetMgr.setUserPresetPath (presetPath.file);

        auto userPresetPath = presetMgr.getUserPresetPath();
        presetFiles.push_back (userPresetPath.getChildFile ("test1.preset"));
        presetMgr.saveUserPreset (presetFiles.back());

        presetFiles.push_back (userPresetPath.getChildFile ("test2.preset"));
        presetMgr.saveUserPreset (presetFiles.back());

        juce::String testUserName = "User";
        presetMgr.setUserPresetName (testUserName);
        expectEquals (presetMgr.getUserPresetName(), testUserName, "Default user name is incorrect!");
        for (const auto* preset : presetMgr.getUserPresets())
            expectEquals (preset->getVendor(), testUserName, "Default user vendor name is incorrect!");

        testUserName = "TestUserName";
        presetMgr.setUserPresetName (testUserName);
        expectEquals (presetMgr.getUserPresetName(), testUserName, "Default user name is incorrect!");
        for (const auto* preset : presetMgr.getUserPresets())
            expectEquals (preset->getVendor(), testUserName, "Set user vendor name is incorrect!");

        presetMgr.setUserPresetName (testUserName);
        expectEquals (presetMgr.getUserPresetName(), testUserName, "Default user name is incorrect!");
        for (const auto* preset : presetMgr.getUserPresets())
            expectEquals (preset->getVendor(), testUserName, "Changing user vendor name that is not default should not change existing user preset names!");

        for (auto& file : presetFiles)
            file.deleteFile();
    }

    void runTestTimed() override
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

        // this test is SUPER slow on Mac, so let's disable it by default
#if ! JUCE_MAC
        beginTest ("Preset State Test");
        presetStateTest();
#endif

        beginTest ("Default Preset Test");
        defaultPresetTest();

        beginTest ("Trigger Preset List Update Test");
        triggerPresetListUpdateTest();

        beginTest ("Get User Presets Test");
        getUserPresetsTest();
    }
};

static PresetManagerTest presetManagerTest;
