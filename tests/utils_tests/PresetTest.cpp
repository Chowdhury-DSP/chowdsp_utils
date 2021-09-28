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

        chowdsp::Preset testPreset (presetName, vendorName, presetXml);
        expect (testPreset.isValid(), "Test preset is not valid!");

        File testFile = File::getSpecialLocation (File::userHomeDirectory).getChildFile ("test.preset");
        testPreset.toFile (testFile);

        chowdsp::Preset filePreset (testFile);
        expect (filePreset.isValid(), "File preset is not valid!");

        auto compareVal = filePreset.getState()->getDoubleAttribute (testTag);
        expectEquals (compareVal, testValue, "Saved value is incorrect!");

        testFile.deleteFile();
    }

    void invalidPresetTest()
    {
        {
            XmlElement* nullXml = nullptr;
            chowdsp::Preset preset { nullXml };
            expect (! preset.isValid(), "Null XML preset should be invalid!");
        }

        {
            auto badXML = std::make_unique<XmlElement> ("BAD_XML");
            chowdsp::Preset preset { badXML.get() };
            expect (! preset.isValid(), "Bad XML preset should be invalid!");
        }

        {
            auto noNameXML = std::make_unique<XmlElement> (chowdsp::Preset::presetTag);
            noNameXML->setAttribute (chowdsp::Preset::nameTag, "");
            chowdsp::Preset preset { noNameXML.get() };
            expect (! preset.isValid(), "XML with no name should be invalid!");
        }

        {
            auto wrongPluginXML = std::make_unique<XmlElement> (chowdsp::Preset::presetTag);
            wrongPluginXML->setAttribute (chowdsp::Preset::nameTag, "Test Preset");
            wrongPluginXML->setAttribute (chowdsp::Preset::pluginTag, "Bad Plugin");
            chowdsp::Preset preset { wrongPluginXML.get() };
            expect (! preset.isValid(), "XML with wrong plugin name should be invalid!");
        }

        {
            auto noVendorXML = std::make_unique<XmlElement> (chowdsp::Preset::presetTag);
            noVendorXML->setAttribute (chowdsp::Preset::nameTag, "Test Preset");
            noVendorXML->setAttribute (chowdsp::Preset::pluginTag, JucePlugin_Name);
            noVendorXML->setAttribute (chowdsp::Preset::vendorTag, "");
            chowdsp::Preset preset { noVendorXML.get() };
            expect (! preset.isValid(), "XML with no vendor should be invalid!");
        }

        {
            auto noVersionXML = std::make_unique<XmlElement> (chowdsp::Preset::presetTag);
            noVersionXML->setAttribute (chowdsp::Preset::nameTag, "Test Preset");
            noVersionXML->setAttribute (chowdsp::Preset::pluginTag, JucePlugin_Name);
            noVersionXML->setAttribute (chowdsp::Preset::vendorTag, "Test Vendor");
            noVersionXML->setAttribute (chowdsp::Preset::versionTag, "");
            chowdsp::Preset preset { noVersionXML.get() };
            expect (! preset.isValid(), "XML with no version should be invalid!");
        }

        {
            auto noStateXML = std::make_unique<XmlElement> (chowdsp::Preset::presetTag);
            noStateXML->setAttribute (chowdsp::Preset::nameTag, "Test Preset");
            noStateXML->setAttribute (chowdsp::Preset::pluginTag, JucePlugin_Name);
            noStateXML->setAttribute (chowdsp::Preset::vendorTag, "Test Vendor");
            noStateXML->setAttribute (chowdsp::Preset::versionTag, JucePlugin_VersionString);
            chowdsp::Preset preset { noStateXML.get() };
            expect (! preset.isValid(), "XML with no state should be invalid!");
        }

        {
            auto goodXML = std::make_unique<XmlElement> (chowdsp::Preset::presetTag);
            goodXML->setAttribute (chowdsp::Preset::nameTag, "Test Preset");
            goodXML->setAttribute (chowdsp::Preset::pluginTag, JucePlugin_Name);
            goodXML->setAttribute (chowdsp::Preset::vendorTag, "Test Vendor");
            goodXML->setAttribute (chowdsp::Preset::versionTag, JucePlugin_VersionString);
            goodXML->addChildElement (new XmlElement ("Test State"));
            chowdsp::Preset preset { goodXML.get() };
            expect (preset.isValid(), "Good preset should be valid!");
        }
    }

    void presetPropertiesTest()
    {
        auto testFile = File::getSpecialLocation (File::userHomeDirectory).getChildFile ("test.preset");
        auto xml = std::make_unique<XmlElement> ("TEST_XML");

        auto testPreset = [=] (const chowdsp::Preset& p) {
            expectEquals (p.getName(), String ("test preset"), "Preset name incorrect!");
            expectEquals (p.getVendor(), String ("test vendor"), "Preset vendor incorrect!");
            expectEquals (p.getCategory(), String ("test category"), "Preset category incorrect!");
            expectEquals (p.getState()->getTagName(), String ("TEST_XML"), "Preset state incorrect!");
            expectEquals (p.getVersion().getVersionString(), String (JucePlugin_VersionString), "Preset version incorrect!");
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

        expectEquals (testPreset.getName(), String ("Test Preset"), "BinaryData preset name is incorrect!");
        expectEquals (testPreset.getVendor(), String ("Factory"), "BinaryData preset vendor is incorrect!");

        auto testValue = testPreset.getState()->getChildByName ("PARAM")->getDoubleAttribute ("value");
        expectEquals (testValue, 1.0, "Preset test value is incorrect!");
    }

    void runTest() override
    {
        beginTest ("File Save/Load Test");
        saveLoadTest();

        beginTest ("Invalid Preset Test");
        invalidPresetTest();

        beginTest ("Preset Properties Test");
        presetPropertiesTest();

        beginTest ("Binary Data Preset Test");
        binaryDataPresetTest();
    }
};

static PresetTest presetTest;
