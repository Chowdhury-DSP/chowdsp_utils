#include <JuceHeader.h>

class PresetTest : public UnitTest
{
public:
    PresetTest() : UnitTest ("Preset Test")
    {
    }

    void saveLoadTest()
    {
        const String presetName = "Test Preset";
        const String vendorName = "Test Vendor";
        const String testTag = "test_attribute";
        constexpr double testValue = 0.5;

        XmlElement presetXml ("Parameters");
        presetXml.setAttribute (testTag, testValue);

        chowdsp::Preset testPreset (presetName, vendorName, std::move (presetXml));
        expect (testPreset.isValid(), "Test preset is not valid!");

        File testFile = File::getSpecialLocation (File::userHomeDirectory).getChildFile ("test.preset");
        testPreset.toFile (testFile);

        chowdsp::Preset filePreset (testFile);
        expect (filePreset.isValid(), "File preset is not valid!");

        auto compareVal = filePreset.state->getDoubleAttribute (testTag);
        expectEquals (compareVal, testValue, "Saved value is incorrect!");

        testFile.deleteFile();
    }

    void invalidPresetTest()
    {
        std::cout << "Invalid Preset Test... TODO" << std::endl;
    }

    void runTest() override
    {
        beginTest ("File Save/Load Test");
        saveLoadTest();

        beginTest ("Invalid Preset Test");
        invalidPresetTest();
    }
};

static PresetTest presetTest;
