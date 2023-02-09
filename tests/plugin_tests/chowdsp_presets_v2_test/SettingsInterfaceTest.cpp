#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

struct SettingsIParams : chowdsp::ParamHolder
{
};

TEST_CASE ("Settings Interface Test", "[plugin][presets][plugin-settings]")
{
    chowdsp::PluginStateImpl<SettingsIParams> state {};
    chowdsp::presets::PresetManager presetMgr { state };

    SECTION ("User Presets Dir Settings")
    {
        chowdsp::GlobalPluginSettings pluginSettings;
        pluginSettings.initialise ("test_settings.json");
        pluginSettings.addProperties ({ { "blah", false } });

        juce::File dummyFile { juce::File::getSpecialLocation (juce::File::userDesktopDirectory).getChildFile ("dummy") };
        dummyFile.create();

        chowdsp::presets::frontend::SettingsInterface settingsInterface { presetMgr,
                                                                          pluginSettings,
                                                                          dummyFile };
        REQUIRE (presetMgr.getUserPresetPath() == dummyFile);

        juce::File dir2 { juce::File::getSpecialLocation (juce::File::userDesktopDirectory).getChildFile ("dummy2") };
        dir2.createDirectory();

        settingsInterface.setUserPresetsPath (dir2);
        REQUIRE (presetMgr.getUserPresetPath() == dir2);

        settingsInterface.setUserPresetsPath (dir2);
        REQUIRE (presetMgr.getUserPresetPath() == dir2);

        pluginSettings.setProperty ("blah", true);
        REQUIRE (presetMgr.getUserPresetPath() == dir2);

        dir2.deleteRecursively();
        dummyFile.deleteRecursively();
        pluginSettings.getSettingsFile().deleteFile();
    }
}
