#include <TimedUnitTest.h>
#include <chowdsp_presets/chowdsp_presets.h>
#include "TestPresetBinaryData.h"

class PresetTest : public TimedUnitTest
{
public:
    PresetTest() : TimedUnitTest ("Preset Test", "Presets")
    {
    }

    void saveLoadTest()
    {
        const juce::String presetName = "Test Preset";
        const juce::String vendorName = "Test Vendor";
        const juce::String testTag = "test_attribute";
        constexpr double testValue = 0.5;

        juce::XmlElement presetXml ("Parameters");
        presetXml.setAttribute (testTag, testValue);

        chowdsp::Preset testPreset (presetName, vendorName, presetXml);
        expect (testPreset.isValid(), "Test preset is not valid!");

        juce::File testFile = juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("test.preset");
        testPreset.toFile (testFile);

        chowdsp::Preset filePreset (testFile);
        expect (filePreset.isValid(), "File preset is not valid!");
        expectEquals (filePreset.getPresetFile().getFullPathName(), testFile.getFullPathName(), "Preset file is incorrect!");

        auto compareVal = filePreset.getState()->getDoubleAttribute (testTag);
        expectEquals (compareVal, testValue, "Saved value is incorrect!");

        testFile.deleteFile();
    }

    void invalidPresetTest()
    {
        {
            juce::XmlElement* nullXml = nullptr;
            chowdsp::Preset preset { nullXml };
            expect (! preset.isValid(), "Null XML preset should be invalid!");

            juce::File testFile = juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("test_null.preset");
            preset.toFile (testFile);
            expect (! testFile.existsAsFile(), "Invalid presets can't be saved to a file!");

            expect (preset.toXml() == nullptr, "Invalid preset should return nullptr XML!");

            chowdsp::Preset testValidPreset { BinaryData::test_preset_preset, BinaryData::test_preset_presetSize };
            expect (preset != testValidPreset, "Invalid preset should not equal valid preset!");
        }

        {
            auto badXML = std::make_unique<juce::XmlElement> ("BAD_XML");
            chowdsp::Preset preset { badXML.get() };
            expect (! preset.isValid(), "Bad XML preset should be invalid!");
        }

        {
            auto noNameXML = std::make_unique<juce::XmlElement> (chowdsp::Preset::presetTag);
            noNameXML->setAttribute (chowdsp::Preset::nameTag, "");
            chowdsp::Preset preset { noNameXML.get() };
            expect (! preset.isValid(), "XML with no name should be invalid!");
        }

        {
            auto wrongPluginXML = std::make_unique<juce::XmlElement> (chowdsp::Preset::presetTag);
            wrongPluginXML->setAttribute (chowdsp::Preset::nameTag, "Test Preset");
            wrongPluginXML->setAttribute (chowdsp::Preset::pluginTag, "Bad Plugin");
            chowdsp::Preset preset { wrongPluginXML.get() };
            expect (! preset.isValid(), "XML with wrong plugin name should be invalid!");
        }

        {
            auto noVendorXML = std::make_unique<juce::XmlElement> (chowdsp::Preset::presetTag);
            noVendorXML->setAttribute (chowdsp::Preset::nameTag, "Test Preset");
            noVendorXML->setAttribute (chowdsp::Preset::pluginTag, JucePlugin_Name);
            noVendorXML->setAttribute (chowdsp::Preset::vendorTag, "");
            chowdsp::Preset preset { noVendorXML.get() };
            expect (! preset.isValid(), "XML with no vendor should be invalid!");
        }

        {
            auto noVersionXML = std::make_unique<juce::XmlElement> (chowdsp::Preset::presetTag);
            noVersionXML->setAttribute (chowdsp::Preset::nameTag, "Test Preset");
            noVersionXML->setAttribute (chowdsp::Preset::pluginTag, JucePlugin_Name);
            noVersionXML->setAttribute (chowdsp::Preset::vendorTag, "Test Vendor");
            noVersionXML->setAttribute (chowdsp::Preset::versionTag, "");
            chowdsp::Preset preset { noVersionXML.get() };
            expect (! preset.isValid(), "XML with no version should be invalid!");
        }

        {
            auto noStateXML = std::make_unique<juce::XmlElement> (chowdsp::Preset::presetTag);
            noStateXML->setAttribute (chowdsp::Preset::nameTag, "Test Preset");
            noStateXML->setAttribute (chowdsp::Preset::pluginTag, JucePlugin_Name);
            noStateXML->setAttribute (chowdsp::Preset::vendorTag, "Test Vendor");
            noStateXML->setAttribute (chowdsp::Preset::versionTag, JucePlugin_VersionString);
            chowdsp::Preset preset { noStateXML.get() };
            expect (! preset.isValid(), "XML with no state should be invalid!");

            chowdsp::Preset testValidPreset { BinaryData::test_preset_preset, BinaryData::test_preset_presetSize };
            expect (preset != testValidPreset, "Invalid preset should not equal valid preset!");
        }

        {
            auto goodXML = std::make_unique<juce::XmlElement> (chowdsp::Preset::presetTag);
            goodXML->setAttribute (chowdsp::Preset::nameTag, "Test Preset");
            goodXML->setAttribute (chowdsp::Preset::pluginTag, JucePlugin_Name);
            goodXML->setAttribute (chowdsp::Preset::vendorTag, "Test Vendor");
            goodXML->setAttribute (chowdsp::Preset::versionTag, JucePlugin_VersionString);
            goodXML->addChildElement (new juce::XmlElement ("Test_State"));
            chowdsp::Preset preset { goodXML.get() };
            expect (preset.isValid(), "Good preset should be valid!");
        }
    }

    void presetPropertiesTest()
    {
        auto testFile = juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("test.preset");
        auto xml = std::make_unique<juce::XmlElement> ("TEST_XML");

        auto testPreset = [=] (const chowdsp::Preset& p)
        {
            expectEquals (p.getName(), juce::String ("test preset"), "Preset name incorrect!");
            expectEquals (p.getVendor(), juce::String ("test vendor"), "Preset vendor incorrect!");
            expectEquals (p.getCategory(), juce::String ("test category"), "Preset category incorrect!");
            expectEquals (p.getState()->getTagName(), juce::String ("TEST_XML"), "Preset state incorrect!");
            expectEquals (p.getVersion().getVersionString(), juce::String (JucePlugin_VersionString), "Preset version incorrect!");
        };

        chowdsp::Preset preset { "test preset", "test vendor", *xml, "test category" };
        testPreset (preset);
        preset.toFile (testFile);

        chowdsp::Preset newPreset { testFile };
        testPreset (newPreset);

        testFile.deleteRecursively();
    }

    void binaryDataPresetTest()
    {
        chowdsp::Preset testPreset { BinaryData::test_preset_preset, BinaryData::test_preset_presetSize };

        expectEquals (testPreset.getName(), juce::String ("Test Preset"), "BinaryData preset name is incorrect!");
        expectEquals (testPreset.getVendor(), juce::String ("Factory"), "BinaryData preset vendor is incorrect!");

        auto testValue = testPreset.getState()->getChildByName ("PARAM")->getDoubleAttribute ("value");
        expectEquals (testValue, 1.0, "Preset test value is incorrect!");
    }

    void extraInfoPresetTest()
    {
        const juce::String presetName = "Test Preset";
        const juce::String vendorName = "Test Vendor";
        const juce::String testTag = "test_attribute";
        const juce::String testTag2 = "test_attribute2";
        constexpr double testValue = 0.5;
        constexpr double testValue2 = 0.99;

        juce::XmlElement presetXml ("Parameters");
        presetXml.setAttribute (testTag, testValue);

        chowdsp::Preset testPreset (presetName, vendorName, presetXml);
        testPreset.extraInfo.setAttribute (testTag2, testValue2);
        expect (testPreset.isValid(), "Test preset is not valid!");

        juce::File testFile = juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("test.preset");
        testPreset.toFile (testFile);

        chowdsp::Preset filePreset (testFile);
        expect (filePreset.isValid(), "File preset is not valid!");

        auto compareVal = filePreset.getState()->getDoubleAttribute (testTag);
        expectEquals (compareVal, testValue, "Saved value is incorrect!");

        auto compareVal2 = filePreset.extraInfo.getDoubleAttribute (testTag2);
        expectEquals (compareVal2, testValue2, "Extra info value is incorrect!");

        testFile.deleteFile();
    }

    void presetsEqualTest()
    {
        chowdsp::Preset testPreset { BinaryData::test_preset_preset, BinaryData::test_preset_presetSize };
        expect (testPreset == testPreset, "Preset should equal itself!");

        const juce::String presetName = "Test Preset";
        const juce::String vendorName = "Test Vendor";
        const juce::String testTag = "test_attribute";
        constexpr double testValue = 0.5;

        juce::XmlElement presetXml ("Parameters");
        presetXml.setAttribute (testTag, testValue);
        chowdsp::Preset testPreset2 (presetName, vendorName, presetXml);
        expect (testPreset != testPreset2, "These presets should not be equivalent!");
    }

    void runTestTimed() override
    {
        beginTest ("File Save/Load Test");
        saveLoadTest();

        beginTest ("Invalid Preset Test");
        invalidPresetTest();

        beginTest ("Preset Properties Test");
        presetPropertiesTest();

        beginTest ("Binary Data Preset Test");
        binaryDataPresetTest();

        beginTest ("Extra Info Preset Test");
        extraInfoPresetTest();

        beginTest ("Presets Equal Test");
        presetsEqualTest();
    }
};

static PresetTest presetTest;
