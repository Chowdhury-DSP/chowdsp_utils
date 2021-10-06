#include "../DummyPlugin.h"
#include "../test_utils.h"

using namespace test_utils;

class PresetsCompTest : public UnitTest
{
public:
    PresetsCompTest() : UnitTest ("Presets Component Test")
    {
    }

    void presetBoxTextTest()
    {
        DummyPlugin plugin { true };
        auto& presetMgr = plugin.getPresetManager();
        auto param = plugin.getParameters()[0];

        ScopedFile presetPath ("preset_path");
        presetPath.file.createDirectory();

        ScopedFile presetFile1 ("preset_path/Test1.preset");
        presetMgr.saveUserPreset (presetFile1.file);

        setParameter (param, 0.1f);
        ScopedFile presetFile2 ("preset_path/Test2.preset");
        presetMgr.saveUserPreset (presetFile2.file);

        chowdsp::PresetsComp presetsComp { presetMgr };
        presetMgr.setUserPresetConfigFile ("preset_config.txt");
        presetMgr.setUserPresetPath (presetPath.file);

        expectEquals (presetsComp.getPresetMenuText(), String ("Test2"), "Initial preset text is incorrect!");

        setParameter (param, 0.9f);
        expectEquals (presetsComp.getPresetMenuText(), String ("Test2*"), "Dirty preset text is incorrect!");

        presetsComp.goToNextPreset (true);
        expectEquals (presetsComp.getPresetMenuText(), String ("Test1"), "Loaded preset text is incorrect!");

        setParameter (param, 0.9f);
        expectEquals (presetsComp.getPresetMenuText(), String ("Test1*"), "Dirty preset text is incorrect!");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
    }

    PopupMenu::Item* getMenuItem (const PopupMenu& menu, const String& itemText)
    {
        auto menuIter = PopupMenu::MenuItemIterator (menu, true);
        while (menuIter.next())
        {
            if (menuIter.getItem().text == itemText)
                return &menuIter.getItem();
        }

        return nullptr;
    }

    void chooserPresetFolderTest()
    {
        DummyPlugin plugin { true };
        auto& presetMgr = plugin.getPresetManager();
        chowdsp::PresetsComp presetsComp { presetMgr };

        const auto* menu = presetsComp.getPresetMenuBox().getRootMenu();
        auto menuItem = getMenuItem (*menu, "Choose Preset folder...");
        menuItem->action();
    }

    void goToPresetFolderTest()
    {
        DummyPlugin plugin;
        ScopedFile presetPath ("preset_path");
        presetPath.file.createDirectory();

        chowdsp::PresetManager presetMgr { plugin.getVTS() };
        presetMgr.setUserPresetConfigFile ("preset_config.txt");
        presetMgr.setUserPresetPath (presetPath.file);

        chowdsp::PresetsComp presetsComp { presetMgr };
        const auto* menu = presetsComp.getPresetMenuBox().getRootMenu();
        auto menuItem = getMenuItem (*menu, "Go to Preset folder...");
        menuItem->action();

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
    }

    void savePresetTest()
    {
        DummyPlugin plugin;
        ScopedFile presetPath ("preset_path");
        presetPath.file.createDirectory();

        chowdsp::PresetManager presetMgr { plugin.getVTS() };
        presetMgr.setUserPresetConfigFile ("preset_config.txt");
        presetMgr.setUserPresetPath (presetPath.file);

        chowdsp::PresetsComp presetsComp { presetMgr };
        const auto* menu = presetsComp.getPresetMenuBox().getRootMenu();
        auto& nameEditor = presetsComp.getPresetNameEditor();
        auto menuItem = getMenuItem (*menu, "Save Preset");

        menuItem->action();
        nameEditor.onEscapeKey();
        expect (nameEditor.isVisible() == false, "Name Editor should be invisible after ESC!");
        expectEquals (presetMgr.getNumPresets(), 0, "Escape key should not save user preset!");

        menuItem->action();
        nameEditor.onReturnKey();
        expect (nameEditor.isVisible() == false, "Name Editor should be invisible after RETURN!");
        expectEquals (presetMgr.getNumPresets(), 1, "Return key did not save user preset!");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
    }

    void loadPresetTest()
    {
        DummyPlugin plugin { true };
        auto& presetMgr = plugin.getPresetManager();
        auto param = plugin.getParameters()[0];

        ScopedFile presetPath ("preset_path");
        presetPath.file.createDirectory();

        ScopedFile presetFile1 ("preset_path/Test1.preset");
        presetMgr.saveUserPreset (presetFile1.file);

        setParameter (param, 0.1f);
        ScopedFile presetFile2 ("preset_path/Test2.preset");
        presetMgr.saveUserPreset (presetFile2.file);

        chowdsp::PresetsComp presetsComp { presetMgr };
        presetMgr.setUserPresetConfigFile ("preset_config.txt");
        presetMgr.setUserPresetPath (presetPath.file);

        expectEquals (presetsComp.getPresetMenuText(), String ("Test2"), "Initial preset text is incorrect!");

        const auto* menu = presetsComp.getPresetMenuBox().getRootMenu();
        auto menuItem = getMenuItem (*menu, "Test1");
        menuItem->action();
        expectEquals (presetsComp.getPresetMenuText(), String ("Test1"), "Loaded preset text is incorrect!");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
    }

    void runTest() override
    {
        beginTest ("Preset Box Text Test");
        presetBoxTextTest();

#if ! JUCE_LINUX
        beginTest ("Choose Preset Folder Test");
        chooserPresetFolderTest();

        beginTest ("Go To Preset Folder Test");
        goToPresetFolderTest();
#endif

        beginTest ("Save Preset Test");
        savePresetTest();

        beginTest ("Load Preset Test");
        loadPresetTest();
    }
};

static PresetsCompTest presetsCompTest;
