#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>
#include "test_utils.h"
#include "TestPresetBinaryData.h"

TEST_CASE ("Preset Test", "[plugin][presets]")
{
    SECTION ("File Save/Load")
    {
        const juce::String presetName = "Test Preset";
        const juce::String vendorName = "Test Vendor";
        const juce::String testTag = "test_attribute";
        constexpr double testValue = 0.5;

        chowdsp::presets::Preset testPreset (presetName, vendorName, { { testTag, testValue } });
        REQUIRE_MESSAGE (testPreset.isValid(), "Test preset is not valid!");

        test_utils::ScopedFile testFile { juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("test.preset") };
        testPreset.toFile (testFile);

        chowdsp::presets::Preset filePreset (testFile);
        REQUIRE_MESSAGE (filePreset.isValid(), "File preset is not valid!");
        REQUIRE_MESSAGE (filePreset.getPresetFile().getFullPathName() == testFile.file.getFullPathName(), "Preset file is incorrect!");

        auto compareVal = filePreset.getState()[testTag].get<float>();
        REQUIRE_MESSAGE (juce::approximatelyEqual (compareVal, (float) testValue), "Saved value is incorrect!");
    }

    SECTION ("Invalid Preset")
    {
        {
            chowdsp::presets::Preset preset { "Name", "Vendor", {} };
            REQUIRE_MESSAGE (! preset.isValid(), "Null JSON preset should be invalid!");

            juce::File testFile = juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("test_null.preset");
            preset.toFile (testFile);
            REQUIRE_MESSAGE (! testFile.existsAsFile(), "Invalid presets can't be saved to a file!");

            chowdsp::presets::Preset testValidPreset { BinaryData::test_preset_preset, BinaryData::test_preset_presetSize };
            REQUIRE_MESSAGE (preset != testValidPreset, "Invalid preset should not equal valid preset!");
        }

        {
            test_utils::ScopedFile presetJson { "bad_file.preset" };
            presetJson.file.replaceWithText ("blah_blah_blah");
            chowdsp::presets::Preset preset { presetJson };
            REQUIRE_MESSAGE (! preset.isValid(), "Bad file preset should be invalid!");
        }

        {
            chowdsp::presets::Preset validPreset { "Name", "Vendor", { { "tag", 0.0f } } };
            const auto validJson = validPreset.toJson();

            auto screwUpJSONTest = [&validJson] (auto&& func, const auto& msg)
            {
                test_utils::ScopedFile presetJson { "bad_preset.preset" };
                auto badJson = validJson;
                func (badJson);
                chowdsp::JSONUtils::toFile (badJson, presetJson);
                chowdsp::presets::Preset invalidPreset { presetJson };
                REQUIRE_MESSAGE (! invalidPreset.isValid(), msg);
            };

            screwUpJSONTest ([] (auto& badJson)
                             { badJson.erase (chowdsp::presets::Preset::nameTag); },
                             "Preset with no name should be invalid!");
            screwUpJSONTest ([] (auto& badJson)
                             { badJson.at (chowdsp::presets::Preset::nameTag) = ""; },
                             "Preset with empty name should be invalid!");
            screwUpJSONTest ([] (auto& badJson)
                             { badJson.erase (chowdsp::presets::Preset::vendorTag); },
                             "Preset with no vendor should be invalid!");
            screwUpJSONTest ([] (auto& badJson)
                             { badJson.at (chowdsp::presets::Preset::vendorTag) = ""; },
                             "Preset with empty vendor should be invalid!");
            screwUpJSONTest ([] (auto& badJson)
                             { badJson.at (chowdsp::presets::Preset::pluginTag) = "other plugin"; },
                             "Preset for wrong plugin should be invalid!");
            screwUpJSONTest ([] (auto& badJson)
                             { badJson.at (chowdsp::presets::Preset::versionTag) = ""; },
                             "Preset with empty version should be invalid!");
            screwUpJSONTest ([] (auto& badJson)
                             { badJson.erase (chowdsp::presets::Preset::stateTag); },
                             "Preset with empty version should be invalid!");
        }
    }

    SECTION ("Preset Properties")
    {
        auto testPreset = [] (const chowdsp::presets::Preset& p)
        {
            REQUIRE_MESSAGE (p.getName() == "Name", "Preset name incorrect!");
            REQUIRE_MESSAGE (p.getVendor() == "Vendor", "Preset vendor incorrect!");
            REQUIRE_MESSAGE (p.getCategory() == "Category", "Preset category incorrect!");
            REQUIRE_MESSAGE (p.getState()["tag"] == 0.0f, "Preset state incorrect!");
            REQUIRE_MESSAGE (p.getVersion().getVersionString() == JucePlugin_VersionString, "Preset version incorrect!");
        };

        chowdsp::presets::Preset preset { "Name", "Vendor", { { "tag", 0.0f } }, "Category" };
        testPreset (preset);

        test_utils::ScopedFile presetFile { "good_preset.preset" };
        preset.toFile (presetFile);

        chowdsp::presets::Preset newPreset { presetFile };
        testPreset (newPreset);
        REQUIRE_MESSAGE (newPreset.getPresetFile() == presetFile, "Preset file incorrect!");
    }

    SECTION ("BinaryData Preset")
    {
        chowdsp::presets::Preset testPreset { BinaryData::test_preset_preset, BinaryData::test_preset_presetSize };

        REQUIRE_MESSAGE (testPreset.getName() == "Name", "BinaryData preset name is incorrect!");
        REQUIRE_MESSAGE (testPreset.getVendor() == "Vendor", "BinaryData preset vendor is incorrect!");
        REQUIRE_MESSAGE (testPreset.getCategory() == "DRUM", "Preset category incorrect!");
        REQUIRE_MESSAGE (testPreset.getState()["tag"] == 0.0f, "Preset state incorrect!");
        REQUIRE_MESSAGE (testPreset.getVersion().getVersionString() == JucePlugin_VersionString, "Preset version incorrect!");
    }

    SECTION ("Presets Equal")
    {
        chowdsp::presets::Preset preset { "Name", "Vendor", { { "tag", 0.0f } }, "Category" };
        REQUIRE_MESSAGE (preset == preset, "Preset should equal itself!");

        chowdsp::presets::Preset otherPreset { "Name", "Vendor", { { "tag", 10.0f } }, "Category" };
        REQUIRE_MESSAGE (preset != otherPreset, "These presets should not be equivalent!");

        chowdsp::presets::Preset nullPreset { "Name", "Vendor", {}, "Category" };
        REQUIRE_MESSAGE (preset != nullPreset, "Preset with null state should never be equivalent!");
    }

    SECTION ("From JSON")
    {
        chowdsp::presets::Preset preset { "Name", "Vendor", { { "tag", 0.0f } } };
        chowdsp::presets::Preset preset2 { preset.toJson() };
        chowdsp::presets::Preset preset3 { nlohmann::json {} };
        REQUIRE (preset == preset2);
        REQUIRE (preset != preset3);
    }
}
