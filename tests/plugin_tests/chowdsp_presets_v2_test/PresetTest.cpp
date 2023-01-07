#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>
#include "test_utils.h"

TEST_CASE ("Preset Test", "[presets]")
{
    SECTION ("File Save/Load")
    {
        const juce::String presetName = "Test Preset";
        const juce::String vendorName = "Test Vendor";
        const juce::String testTag = "test_attribute";
        constexpr double testValue = 0.5;

        chowdsp::Preset testPreset (presetName, vendorName, { { testTag, testValue } });
        REQUIRE_MESSAGE (testPreset.isValid(), "Test preset is not valid!");

        test_utils::ScopedFile testFile { juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("test.preset") };
        testPreset.toFile (testFile);

        chowdsp::Preset filePreset (testFile);
        REQUIRE_MESSAGE (filePreset.isValid(), "File preset is not valid!");
        REQUIRE_MESSAGE (filePreset.getPresetFile().getFullPathName() == testFile.file.getFullPathName(), "Preset file is incorrect!");

        auto compareVal = filePreset.getState()[testTag].get<float>();
        REQUIRE_MESSAGE (compareVal == testValue, "Saved value is incorrect!");
    }

    SECTION ("Invalid Preset")
    {
        {
            chowdsp::Preset preset { "Name", "Vendor", {} };
            REQUIRE_MESSAGE (! preset.isValid(), "Null JSON preset should be invalid!");

            juce::File testFile = juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("test_null.preset");
            preset.toFile (testFile);
            REQUIRE_MESSAGE (! testFile.existsAsFile(), "Invalid presets can't be saved to a file!");

            //            chowdsp::Preset testValidPreset { BinaryData::test_preset_preset, BinaryData::test_preset_presetSize };
            //            expect (preset != testValidPreset, "Invalid preset should not equal valid preset!");
        }

        {
            test_utils::ScopedFile presetJson { "bad_file.preset" };
            presetJson.file.replaceWithText ("blah_blah_blah");
            chowdsp::Preset preset { presetJson };
            REQUIRE_MESSAGE (! preset.isValid(), "Bad file preset should be invalid!");
        }

        {
            chowdsp::Preset validPreset { "Name", "Vendor", { { "tag", 0.0f } } };
            const auto validJson = validPreset.toJson();

            auto screwUpJSONTest = [&validJson] (auto&& func, const auto& msg)
            {
                test_utils::ScopedFile presetJson { "bad_preset.preset" };
                auto badJson = validJson;
                func (badJson);
                chowdsp::JSONUtils::toFile (badJson, presetJson);
                chowdsp::Preset invalidPreset { presetJson };
                REQUIRE_MESSAGE (! invalidPreset.isValid(), msg);
            };

            screwUpJSONTest ([] (auto& badJson)
                             { badJson.erase (chowdsp::Preset::nameTag); },
                             "Preset with no name should be invalid!");
            screwUpJSONTest ([] (auto& badJson)
                             { badJson.at (chowdsp::Preset::nameTag) = ""; },
                             "Preset with empty name should be invalid!");
            screwUpJSONTest ([] (auto& badJson)
                             { badJson.erase (chowdsp::Preset::vendorTag); },
                             "Preset with no vendor should be invalid!");
            screwUpJSONTest ([] (auto& badJson)
                             { badJson.at (chowdsp::Preset::vendorTag) = ""; },
                             "Preset with empty vendor should be invalid!");
            screwUpJSONTest ([] (auto& badJson)
                             { badJson.at (chowdsp::Preset::pluginTag) = "other plugin"; },
                             "Preset for wrong plugin should be invalid!");
            screwUpJSONTest ([] (auto& badJson)
                             { badJson.at (chowdsp::Preset::versionTag) = ""; },
                             "Preset with empty version should be invalid!");
            screwUpJSONTest ([] (auto& badJson)
                             { badJson.erase (chowdsp::Preset::stateTag); },
                             "Preset with empty version should be invalid!");
        }
    }

    SECTION ("Preset Properties")
    {
        auto testPreset = [] (const chowdsp::Preset& p)
        {
            REQUIRE_MESSAGE (p.getName() == juce::String ("Name"), "Preset name incorrect!");
            REQUIRE_MESSAGE (p.getVendor() == juce::String ("Vendor"), "Preset vendor incorrect!");
            REQUIRE_MESSAGE (p.getCategory() == juce::String ("Category"), "Preset category incorrect!");
            REQUIRE_MESSAGE (p.getState()["tag"] == 0.0f, "Preset state incorrect!");
            REQUIRE_MESSAGE (p.getVersion().getVersionString() == juce::String (JucePlugin_VersionString), "Preset version incorrect!");
        };

        chowdsp::Preset preset { "Name", "Vendor", { { "tag", 0.0f } }, "Category" };
        testPreset (preset);

        test_utils::ScopedFile presetFile { "good_preset.preset" };
        preset.toFile (presetFile);

        chowdsp::Preset newPreset { presetFile };
        testPreset (newPreset);
        REQUIRE_MESSAGE (newPreset.getPresetFile() == presetFile, "Preset file incorrect!");
    }

    //    void binaryDataPresetTest()
    //    {
    //        chowdsp::Preset testPreset { BinaryData::test_preset_preset, BinaryData::test_preset_presetSize };
    //
    //        expectEquals (testPreset.getName(), juce::String ("Test Preset"), "BinaryData preset name is incorrect!");
    //        expectEquals (testPreset.getVendor(), juce::String ("Factory"), "BinaryData preset vendor is incorrect!");
    //
    //        auto testValue = testPreset.getState()->getChildByName ("PARAM")->getDoubleAttribute ("value");
    //        expectEquals (testValue, 1.0, "Preset test value is incorrect!");
    //    }

    SECTION ("Extra Info")
    {
        test_utils::ScopedFile presetFile { "extra_info.preset" };
        {
            chowdsp::Preset preset { "Name", "Vendor", { { "tag", 0.0f } } };
            preset.getExtraInfo() = { { "tag2", "something" } };
            preset.toFile (presetFile);
        }

        const chowdsp::Preset preset { presetFile };
        const juce::String extraInfoValue = preset.getExtraInfo().at ("tag2");
        REQUIRE_MESSAGE (extraInfoValue == "something", "Extra info state is incorrect!");
    }

    SECTION ("Presets Equal")
    {
        chowdsp::Preset preset { "Name", "Vendor", { { "tag", 0.0f } }, "Category" };
        REQUIRE_MESSAGE (preset == preset, "Preset should equal itself!");

        chowdsp::Preset otherPreset { "Name", "Vendor", { { "tag", 10.0f } }, "Category" };
        REQUIRE_MESSAGE (preset != otherPreset, "These presets should not be equivalent!");

        chowdsp::Preset nullPreset { "Name", "Vendor", {}, "Category" };
        REQUIRE_MESSAGE (preset != nullPreset, "Preset with null state should never be equivalent!");
    }
}
