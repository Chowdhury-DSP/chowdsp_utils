#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>
#include "test_utils.h"

template <bool extraParameter>
struct Params : chowdsp::ParamHolder
{
    Params()
    {
        add (boolParam, floatParam);
        if constexpr (extraParameter)
            add (extraParam);
    }
    chowdsp::BoolParameter::Ptr boolParam { "bool", "Param", true };
    chowdsp::PercentParameter::Ptr floatParam { "float", "Param", 0.5f };
    chowdsp::PercentParameter::Ptr extraParam { "extra", "Param", 0.0f };
};

template <bool extraParameter = false>
struct ScopedPresetManager
{
    explicit ScopedPresetManager (const juce::String& presetConfigFile = {})
    {
        if (presetConfigFile.isEmpty())
        {
            manager.setUserPresetConfigFile ("preset_config.txt");
            manager.setUserPresetPath (presetPath.file);
        }
        else
        {
            manager.setUserPresetConfigFile (presetConfigFile);
        }
    }

    ~ScopedPresetManager()
    {
        manager.getUserPresetConfigFile().deleteFile();
    }

    auto getPresetFile (const juce::String& path)
    {
        return test_utils::ScopedFile { presetPath.file.getFileName() + "/" + path };
    }

    void loadPreset (int index)
    {
        manager.loadPreset (*manager.getPresetTree().getPresetByIndex (index));
    }

    void setFloatParam (float newValue)
    {
        chowdsp::ParameterTypeHelpers::setValue (newValue, *state.params.floatParam);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (20);
    }

    float getFloatParam() const
    {
        return state.params.floatParam->get();
    }

    void toggleBoolParam()
    {
        chowdsp::ParameterTypeHelpers::setValue (! state.params.boolParam->get(), *state.params.boolParam);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (20);
    }

    operator chowdsp::PresetManager&() { return manager; } // NOLINT
    chowdsp::PresetManager* operator->() { return &manager; }
    const chowdsp::PresetManager* operator->() const { return &manager; }

    chowdsp::PluginStateImpl<Params<extraParameter>> state;
    chowdsp::PresetManager manager { state, nullptr, { state.params.boolParam.get() } };
    test_utils::ScopedFile presetPath { "preset_path" };
};

TEST_CASE ("Preset Manager Test", "[presets][state]")
{
    SECTION ("Save/Load User Presets")
    {
        ScopedPresetManager presetMgr {};

        static constexpr float testValue = 0.45f;
        static constexpr float dummyValue = 0.75f;

        const auto presetFile = presetMgr.getPresetFile ("test.preset");
        presetMgr.setFloatParam (testValue);
        presetMgr->saveUserPreset (presetFile);

        presetMgr.setFloatParam (dummyValue);
        REQUIRE_MESSAGE (presetMgr.getFloatParam() == dummyValue, "Changed value is incorrect!");

        presetMgr.loadPreset (0);
        REQUIRE_MESSAGE (presetMgr.getFloatParam() == testValue, "Preset value is incorrect!");
    }

    SECTION ("Factory Presets")
    {
        static constexpr float testValue = 0.05f;

        std::vector<chowdsp::Preset> presets;
        {
            ScopedPresetManager presetMgr {};
            presetMgr.setFloatParam (testValue);

            const auto presetFile = presetMgr.getPresetFile ("test.preset");
            presetMgr->saveUserPreset (presetFile);
            presets.emplace_back (presetFile);
        }

        ScopedPresetManager presetMgr {};
        presetMgr->addPresets (std::move (presets));

        presetMgr.loadPreset (0);
        REQUIRE_MESSAGE (presetMgr.getFloatParam() == testValue, "Preset value is incorrect!");
    }

    SECTION ("Preset Agnostic Params")
    {
        ScopedPresetManager presetMgr {};
        REQUIRE_MESSAGE (presetMgr.state.params.boolParam->get(), "Initial value incorrect!");

        const auto presetFile1 = presetMgr.getPresetFile ("test.preset");
        presetMgr->saveUserPreset (presetFile1);
        REQUIRE_MESSAGE (presetMgr.state.params.boolParam->get(), "Value after preset save incorrect!");

        presetMgr.toggleBoolParam();
        REQUIRE_MESSAGE (! presetMgr.state.params.boolParam->get(), "Value after toggle incorrect!");

        presetMgr.loadPreset (0);
        REQUIRE_MESSAGE (! presetMgr.state.params.boolParam->get(), "Value after preset load incorrect!");
    }

    SECTION ("Added Params")
    {
        static constexpr float testValue = 0.05f;
        static constexpr float extraValue = 1.0f;
        static constexpr float defaultValue = 0.0f;

        std::vector<chowdsp::Preset> presets;
        {
            ScopedPresetManager presetMgr {};
            presetMgr.setFloatParam (testValue);

            const auto presetFile = presetMgr.getPresetFile ("test.preset");
            presetMgr->saveUserPreset (presetFile);
            presets.emplace_back (presetFile);
        }

        ScopedPresetManager<true> presetMgr {};
        REQUIRE_MESSAGE (presetMgr.state.params.extraParam->get() == defaultValue, "Initial value is incorrect!");

        chowdsp::ParameterTypeHelpers::setValue (extraValue, *presetMgr.state.params.extraParam);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (20);
        REQUIRE_MESSAGE (presetMgr.state.params.extraParam->get() == extraValue, "Set value is incorrect!");
        REQUIRE_MESSAGE (presetMgr->isPresetDirty, "Preset dirty after set value is incorrect!");

        presetMgr->addPresets (std::move (presets));
        presetMgr.loadPreset (0);
        REQUIRE_MESSAGE (presetMgr.state.params.extraParam->get() == defaultValue, "Reset value is incorrect!");
    }

    SECTION ("Dirty Presets")
    {
        static constexpr float initialValue = 0.5f;
        static constexpr float testValue1 = 0.25f;
        static constexpr float testValue2 = 0.85f;

        ScopedPresetManager presetMgr {};
        REQUIRE_MESSAGE (! presetMgr->isPresetDirty, "Initial dirty state is incorrect!");

        const auto presetFile1 = presetMgr.getPresetFile ("test.preset");
        presetMgr->saveUserPreset (presetFile1);
        REQUIRE_MESSAGE (! presetMgr->isPresetDirty, "Dirty state after saving first preset is incorrect!");

        presetMgr.setFloatParam (testValue1);
        REQUIRE_MESSAGE (presetMgr->isPresetDirty, "Dirty state after changing value is incorrect!");

        presetMgr.setFloatParam (initialValue);
        REQUIRE_MESSAGE (presetMgr->isPresetDirty, "Dirty state after return to initial value is incorrect!");

        presetMgr.setFloatParam (testValue2);
        const auto presetFile2 = presetMgr.getPresetFile ("test2.preset");
        presetMgr->saveUserPreset (presetFile2);
        REQUIRE_MESSAGE (! presetMgr->isPresetDirty, "Dirty state after saving second preset is incorrect!");

        presetMgr.setFloatParam (testValue2);
        REQUIRE_MESSAGE (! presetMgr->isPresetDirty, "Dirty state after staying at initial value is incorrect!");

        presetMgr.toggleBoolParam();
        REQUIRE_MESSAGE (! presetMgr->isPresetDirty, "Dirty state after changing preset-agnostic parameter is incorrect!");

        presetMgr.setFloatParam (testValue1);
        REQUIRE_MESSAGE (presetMgr->isPresetDirty, "Dirty state after moving from initial value is incorrect!");
    }

    SECTION ("Default Preset")
    {
        static constexpr float testValue = 0.05f;
        static constexpr float otherValue = 0.75f;

        std::vector<chowdsp::Preset> presets;
        {
            ScopedPresetManager presetMgr {};
            presetMgr.setFloatParam (testValue);

            const auto presetFile = presetMgr.getPresetFile ("test.preset");
            presetMgr->saveUserPreset (presetFile);
            presetMgr->setDefaultPreset (chowdsp::Preset { presetFile });
            REQUIRE_MESSAGE (chowdsp::Preset { presetFile } == *presetMgr->getDefaultPreset(), "Default preset is incorrect!");

            presets.emplace_back (chowdsp::Preset { presetFile });
        }

        ScopedPresetManager presetMgr {};
        presetMgr->setDefaultPreset (std::move (presets[0]));

        presetMgr.setFloatParam (otherValue);
        presetMgr->loadDefaultPreset();
        REQUIRE_MESSAGE (presetMgr.getFloatParam() == testValue, "Preset value is incorrect!");
    }

    SECTION ("User Presets")
    {
        static constexpr float testValue = 0.05f;

        std::vector<chowdsp::Preset> presets;
        juce::File presetConfigFile {};
        {
            ScopedPresetManager presetMgr {};
            presetMgr.setFloatParam (testValue);

            const auto presetFile = presetMgr.getPresetFile ("test.preset");
            presetMgr->saveUserPreset (presetFile);
            presetMgr->setDefaultPreset (chowdsp::Preset { presetFile });
            REQUIRE_MESSAGE (chowdsp::Preset { presetFile } == *presetMgr->getDefaultPreset(), "Default preset is incorrect!");

            presets.emplace_back (chowdsp::Preset { presetFile });
            presetConfigFile = presetMgr->getUserPresetConfigFile();
        }

        test_utils::ScopedFile userPresetsDir { "user_presets" };
        userPresetsDir.file.createDirectory();
        for (auto& preset : presets)
            preset.toFile (userPresetsDir.file.getChildFile (preset.getName()));

        presetConfigFile.create();
        presetConfigFile.replaceWithText (userPresetsDir.file.getFullPathName());

        const ScopedPresetManager presetMgr { presetConfigFile.getFileName() };
        REQUIRE (*presetMgr->getPresetTree().getPresetByIndex (0) == presets[0]);
    }
}
