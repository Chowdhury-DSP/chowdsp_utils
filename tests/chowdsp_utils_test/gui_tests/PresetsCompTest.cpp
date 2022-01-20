#include <DummyPlugin.h>
#include <test_utils.h>
#include <TimedUnitTest.h>

using namespace test_utils;

class PresetsCompTest : public TimedUnitTest
{
public:
    PresetsCompTest() : TimedUnitTest ("Presets Component Test", "Presets")
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

        MessageManager::getInstance()->runDispatchLoopUntil (75);
        expectEquals (presetsComp.getPresetMenuText(), String ("Test2"), "Initial preset text is incorrect!");

        setParameter (param, 0.9f);
        MessageManager::getInstance()->runDispatchLoopUntil (75);
        expectEquals (presetsComp.getPresetMenuText(), String ("Test2*"), "Dirty preset text is incorrect!");

        presetsComp.goToNextPreset (true);
        MessageManager::getInstance()->runDispatchLoopUntil (75);
        expectEquals (presetsComp.getPresetMenuText(), String ("Test1"), "Loaded preset text is incorrect!");

        setParameter (param, 0.9f);
        MessageManager::getInstance()->runDispatchLoopUntil (75);
        expectEquals (presetsComp.getPresetMenuText(), String ("Test1*"), "Dirty preset text is incorrect!");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
    }

    static PopupMenu::Item* getMenuItem (const PopupMenu& menu, const String& itemText)
    {
        auto menuIter = PopupMenu::MenuItemIterator (menu, true);
        while (menuIter.next())
        {
            if (menuIter.getItem().text == itemText)
                return &menuIter.getItem();
        }

        return nullptr;
    }

    static void chooserPresetFolderTest()
    {
        DummyPlugin plugin { true };
        auto& presetMgr = plugin.getPresetManager();
        chowdsp::PresetsComp presetsComp { presetMgr };

        const auto* menu = presetsComp.getPresetMenuBox().getRootMenu();
        auto menuItem = getMenuItem (*menu, "Choose Preset folder...");
        menuItem->action();
    }

    static void goToPresetFolderTest()
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
        expect (! nameEditor.isVisible(), "Name Editor should be invisible after ESC!");
        expectEquals (presetMgr.getNumPresets(), 0, "Escape key should not save user preset!");

        menuItem->action();
        nameEditor.onReturnKey();
        expect (! nameEditor.isVisible(), "Name Editor should be invisible after RETURN!");
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

        MessageManager::getInstance()->runDispatchLoopUntil (75);
        expectEquals (presetsComp.getPresetMenuText(), String ("Test2"), "Initial preset text is incorrect!");

        const auto* menu = presetsComp.getPresetMenuBox().getRootMenu();
        auto menuItem = getMenuItem (*menu, "Test1");
        menuItem->action();
        MessageManager::getInstance()->runDispatchLoopUntil (75);
        expectEquals (presetsComp.getPresetMenuText(), String ("Test1"), "Loaded preset text is incorrect!");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
    }

    void backgroundThreadTest()
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

        std::atomic<bool> threadFinished { false };
        Thread::launch ([&]
                        {
            Thread::sleep (75); // wait for message manager...
            expectEquals (presetsComp.getPresetMenuText(), String ("Test2"), "Initial preset text is incorrect!");

            const auto* menu = presetsComp.getPresetMenuBox().getRootMenu();
            if (auto* menuItem = getMenuItem (*menu, "Test1"))
                menuItem->action();
            Thread::sleep (75); // wait for message manager...
            expectEquals (presetsComp.getPresetMenuText(), String ("Test1"), "Loaded preset text is incorrect!");

            threadFinished = true; });

        while (! threadFinished)
            MessageManager::getInstance()->runDispatchLoopUntil (75);

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
    }

    void runTestTimed() override
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

        beginTest ("Background Thread Test");
        backgroundThreadTest();
    }
};

static PresetsCompTest presetsCompTest;
