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
    explicit ScopedPresetManager (const juce::File& userPresetsPath = {})
    {
        if (userPresetsPath == juce::File {})
            manager.setUserPresetPath (presetPath.file);
        else
            manager.setUserPresetPath (userPresetsPath);
    }

    ~ScopedPresetManager() = default;

    auto getPresetFile (const juce::String& path)
    {
        return test_utils::ScopedFile { presetPath.file.getChildFile (path).withFileExtension (manager.getPresetFileExtension()) };
    }

    void setFloatParam (float newValue)
    {
        chowdsp::ParameterTypeHelpers::setValue (newValue, *state.params.floatParam);
        state.getParameterListeners().updateBroadcastersFromMessageThread();
    }

    float getFloatParam() const
    {
        return state.params.floatParam->get();
    }

    void toggleBoolParam()
    {
        chowdsp::ParameterTypeHelpers::setValue (! state.params.boolParam->get(), *state.params.boolParam);
        state.getParameterListeners().updateBroadcastersFromMessageThread();
    }

    operator chowdsp::presets::PresetManager&() { return manager; } // NOLINT
    chowdsp::presets::PresetManager* operator->() { return &manager; }
    const chowdsp::presets::PresetManager* operator->() const { return &manager; }

    chowdsp::PluginStateImpl<Params<extraParameter>> state;
    chowdsp::presets::PresetManager manager { state, nullptr, ".preset", { state.params.boolParam.get() } };
    test_utils::ScopedFile presetPath { "preset_path" };
};

static chowdsp::presets::Preset saveUserPreset (const juce::String& file, float val, bool makeDefault = false)
{
    ScopedPresetManager presetMgr {};
    presetMgr.setFloatParam (val);
    const auto presetFile = presetMgr.getPresetFile (file);
    presetMgr->saveUserPreset (presetFile);

    if (makeDefault)
    {
        presetMgr->setDefaultPreset (chowdsp::presets::Preset { presetFile });
        REQUIRE_MESSAGE (chowdsp::presets::Preset { presetFile } == *presetMgr->getDefaultPreset(), "Default preset is incorrect!");
    }

    return chowdsp::presets::Preset { presetFile };
}

TEST_CASE ("Preset Manager Test", "[plugin][presets][state]")
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
        REQUIRE_MESSAGE (juce::approximatelyEqual (presetMgr.getFloatParam(), dummyValue), "Changed value is incorrect!");

        presetMgr->loadPreset (presetMgr->getPresetTree().getRootNode().first_child->value.leaf());
        REQUIRE_MESSAGE (juce::approximatelyEqual (presetMgr.getFloatParam(), testValue), "Preset value is incorrect!");
    }

    SECTION ("Factory Presets")
    {
        static constexpr float testValue = 0.05f;
        auto preset = saveUserPreset ("test.preset", testValue);

        ScopedPresetManager presetMgr {};
        presetMgr->addPresets ({ preset });
        presetMgr->loadPreset (presetMgr->getPresetTree().getRootNode().first_child->value.leaf());
        REQUIRE_MESSAGE (juce::approximatelyEqual (presetMgr.getFloatParam(), testValue), "Preset value is incorrect!");
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

        presetMgr->loadPreset (presetMgr->getPresetTree().getRootNode().first_child->value.leaf());
        REQUIRE_MESSAGE (! presetMgr.state.params.boolParam->get(), "Value after preset load incorrect!");
    }

    SECTION ("Added Params")
    {
        static constexpr float testValue = 0.05f;
        static constexpr float extraValue = 1.0f;
        static constexpr float defaultValue = 0.0f;

        auto preset = saveUserPreset ("test.preset", testValue);

        ScopedPresetManager<true> presetMgr {};
        REQUIRE_MESSAGE (juce::approximatelyEqual (presetMgr.state.params.extraParam->get(), defaultValue), "Initial value is incorrect!");

        chowdsp::ParameterTypeHelpers::setValue (extraValue, *presetMgr.state.params.extraParam);
        presetMgr.state.getParameterListeners().updateBroadcastersFromMessageThread();
        REQUIRE_MESSAGE (juce::approximatelyEqual (presetMgr.state.params.extraParam->get(), extraValue), "Set value is incorrect!");
        REQUIRE_MESSAGE (presetMgr->getIsPresetDirty(), "Preset dirty after set value is incorrect!");

        presetMgr->addPresets ({ preset });
        presetMgr->loadPreset (presetMgr->getPresetTree().getRootNode().first_child->value.leaf());
        REQUIRE_MESSAGE (juce::approximatelyEqual (presetMgr.state.params.extraParam->get(), defaultValue), "Reset value is incorrect!");
    }

    SECTION ("Dirty Presets")
    {
        static constexpr float initialValue = 0.5f;
        static constexpr float testValue1 = 0.25f;
        static constexpr float testValue2 = 0.85f;

        ScopedPresetManager presetMgr {};
        REQUIRE_MESSAGE (! presetMgr->getIsPresetDirty(), "Initial dirty state is incorrect!");

        const auto presetFile1 = presetMgr.getPresetFile ("test.preset");
        presetMgr->saveUserPreset (presetFile1);
        REQUIRE_MESSAGE (! presetMgr->getIsPresetDirty(), "Dirty state after saving first preset is incorrect!");

        presetMgr.setFloatParam (testValue1);
        REQUIRE_MESSAGE (presetMgr->getIsPresetDirty(), "Dirty state after changing value is incorrect!");

        presetMgr.setFloatParam (initialValue);
        REQUIRE_MESSAGE (presetMgr->getIsPresetDirty(), "Dirty state after return to initial value is incorrect!");

        presetMgr.setFloatParam (testValue2);
        const auto presetFile2 = presetMgr.getPresetFile ("test2.preset");
        presetMgr->saveUserPreset (presetFile2);
        REQUIRE_MESSAGE (! presetMgr->getIsPresetDirty(), "Dirty state after saving second preset is incorrect!");

        presetMgr.setFloatParam (testValue2);
        REQUIRE_MESSAGE (! presetMgr->getIsPresetDirty(), "Dirty state after staying at initial value is incorrect!");

        presetMgr.toggleBoolParam();
        REQUIRE_MESSAGE (! presetMgr->getIsPresetDirty(), "Dirty state after changing preset-agnostic parameter is incorrect!");

        presetMgr.setFloatParam (testValue1);
        REQUIRE_MESSAGE (presetMgr->getIsPresetDirty(), "Dirty state after moving from initial value is incorrect!");
    }

    SECTION ("Default Preset")
    {
        static constexpr float testValue = 0.05f;
        static constexpr float otherValue = 0.75f;

        auto preset = saveUserPreset ("test.preset", testValue, true);

        ScopedPresetManager presetMgr {};
        presetMgr->setDefaultPreset (std::move (preset));

        presetMgr.setFloatParam (otherValue);
        presetMgr->loadDefaultPreset();
        REQUIRE_MESSAGE (juce::approximatelyEqual (presetMgr.getFloatParam(), testValue), "Preset value is incorrect!");
    }

    SECTION ("User Presets")
    {
        static constexpr float testValue1 = 0.05f;
        static constexpr float testValue2 = 0.55f;

        auto preset1 = saveUserPreset ("test1.preset", testValue1);
        auto preset2 = saveUserPreset ("test2.preset", testValue2);

        test_utils::ScopedFile userPresetsDir1 { "user_presets1" };
        userPresetsDir1.file.createDirectory();
        preset1.toFile (userPresetsDir1.file.getChildFile (preset1.getPresetFile().getFileName()));
        juce::File dummyFile { preset1.getPresetFile().getSiblingFile ("DUMMY") };
        dummyFile.create();

        ScopedPresetManager presetMgr { userPresetsDir1 };
        // REQUIRE_MESSAGE (*presetMgr->getPresetTree().getElementByIndex (0) == preset1, "User preset loaded from folder is incorrect!");

        presetMgr->setUserPresetPath ({});
        // REQUIRE_MESSAGE (*presetMgr->getPresetTree().getElementByIndex (0) == preset1, "User presets should not change when loading null file as preset path!");

        test_utils::ScopedFile userPresetsDir2 { "user_presets2" };
        userPresetsDir2.file.createDirectory();
        preset2.toFile (userPresetsDir2.file.getChildFile (preset2.getPresetFile().getFileName()));
        presetMgr->setUserPresetPath (userPresetsDir2);
        // REQUIRE_MESSAGE (*presetMgr->getPresetTree().getElementByIndex (0) == preset2, "User presets not loaded correctly after changing user preset path!");
    }

    SECTION ("Null State Test")
    {
        static constexpr float otherValue = 0.15f;

        juce::MemoryBlock state;
        {
            ScopedPresetManager presetMgr {};

            presetMgr.setFloatParam (otherValue);
            REQUIRE (juce::approximatelyEqual (presetMgr.getFloatParam(), otherValue));

            presetMgr.state.serialize (state);
        }

        ScopedPresetManager presetMgr {};
        presetMgr.state.deserialize (state);
        REQUIRE_MESSAGE (juce::approximatelyEqual (presetMgr.getFloatParam(), otherValue), "Preset state is overriding parameter state!");
        REQUIRE (presetMgr->getCurrentPreset() == nullptr);
        REQUIRE (presetMgr->getIsPresetDirty());
    }

    SECTION ("Preset State Test")
    {
        static constexpr float testValue = 0.05f;
        static constexpr float otherValue = 0.15f;
        auto preset = saveUserPreset ("test.preset", testValue);

        juce::MemoryBlock state;
        {
            ScopedPresetManager presetMgr {};
            presetMgr->addPresets ({ chowdsp::presets::Preset { preset } });
            presetMgr->loadPreset (presetMgr->getPresetTree().getRootNode().first_child->value.leaf());
            REQUIRE (juce::approximatelyEqual (presetMgr.getFloatParam(), testValue));

            presetMgr.setFloatParam (otherValue);
            REQUIRE (juce::approximatelyEqual (presetMgr.getFloatParam(), otherValue));

            presetMgr.state.serialize (state);
        }

        ScopedPresetManager presetMgr {};
        presetMgr.state.deserialize (state);
        REQUIRE_MESSAGE (juce::approximatelyEqual (presetMgr.getFloatParam(), otherValue), "Preset state is overriding parameter state!");
        REQUIRE (*presetMgr->getCurrentPreset() == preset);
        REQUIRE (presetMgr->getIsPresetDirty());
    }

    SECTION ("Preset Undo/Redo Test")
    {
        static constexpr float testValue = 0.05f;
        static constexpr float testValue2 = 0.15f;
        static constexpr float dirtyVal = 0.22f;
        auto preset = saveUserPreset ("test.preset", testValue);
        auto preset2 = saveUserPreset ("test1.preset", testValue2);

        juce::UndoManager um { 1000 };
        ScopedPresetManager presetMgr {};
        presetMgr.state.undoManager = &um;

        presetMgr->addPresets ({ chowdsp::presets::Preset { preset }, chowdsp::presets::Preset { preset2 } });
        presetMgr->loadPreset (presetMgr->getPresetTree().getRootNode().first_child->value.leaf());
        REQUIRE (juce::approximatelyEqual (presetMgr.getFloatParam(), testValue));

        presetMgr.setFloatParam (dirtyVal);
        REQUIRE (juce::approximatelyEqual (presetMgr.getFloatParam(), dirtyVal));
        REQUIRE (presetMgr->getIsPresetDirty());

        presetMgr->loadPreset (presetMgr->getPresetTree().getRootNode().first_child->next_sibling->value.leaf());
        REQUIRE (juce::approximatelyEqual (presetMgr.getFloatParam(), testValue2));
        REQUIRE (! presetMgr->getIsPresetDirty());

        REQUIRE (um.canUndo());
        REQUIRE (um.undo());
        REQUIRE (presetMgr->getIsPresetDirty());
        REQUIRE (juce::approximatelyEqual (presetMgr.getFloatParam(), dirtyVal));

        REQUIRE (um.canRedo());
        REQUIRE (um.redo());
        REQUIRE (! presetMgr->getIsPresetDirty());
        REQUIRE (juce::approximatelyEqual (presetMgr.getFloatParam(), testValue2));
    }
}
