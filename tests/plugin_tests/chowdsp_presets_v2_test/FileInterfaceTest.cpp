#include <CatchUtils.h>
#include <test_utils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>

struct FileIParams : chowdsp::ParamHolder
{
    FileIParams()
    {
        add (floatParam);
    }
    chowdsp::PercentParameter::Ptr floatParam { "float", "Param", 0.5f };
};

TEST_CASE ("File Interface Test", "[plugin][presets]")
{
    chowdsp::PluginStateImpl<FileIParams> state {};
    chowdsp::presets::PresetManager presetMgr { state };
    chowdsp::presets::frontend::FileInterface fileInterface { presetMgr };

    SECTION ("Save Preset")
    {
        bool called = false;
        fileInterface.savePresetCallback = [&called] (auto&&)
        {
            called = true;
        };

        fileInterface.savePreset();
        REQUIRE (called);
    }

    SECTION ("Resave Current Preset")
    {
        test_utils::ScopedFile file { "preset.preset" };
        {
            chowdsp::presets::Preset preset { "Name", "Vendor", { { "float", 0.0f } } };
            preset.toFile (file);

            presetMgr.setDefaultPreset (std::move (preset));
            presetMgr.loadDefaultPreset();

            chowdsp::ParameterTypeHelpers::setValue (1.0f, *state.params.floatParam);
            fileInterface.resaveCurrentPreset();
        }

        chowdsp::presets::Preset preset { file };
        REQUIRE (preset.getState()["float"] == 1.0f);
    }

    SECTION ("Delete Current Preset")
    {
        test_utils::ScopedFile file { "preset.preset" };
        chowdsp::presets::Preset preset { "Name", "Vendor", { { "float", 0.0f } } };
        preset.toFile (file);

        presetMgr.setDefaultPreset (std::move (preset));
        presetMgr.loadDefaultPreset();

        fileInterface.confirmAndDeletePresetCallback = [] (const chowdsp::presets::Preset& p,
                                                           std::function<void (const chowdsp::presets::Preset&)>&& presetDeleter)
        {
            presetDeleter (p);
        };
        fileInterface.deleteCurrentPreset();
        REQUIRE (! file.file.existsAsFile());
    }
}
