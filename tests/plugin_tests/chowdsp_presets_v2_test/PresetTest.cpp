#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>
#include "test_utils.h"

TEST_CASE ("Preset Test", "[presets]")
{
    SECTION ("File Save/Load Test")
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
}
