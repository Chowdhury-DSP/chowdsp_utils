#include <DummyPlugin.h>
#include <test_utils.h>
#include <TimedUnitTest.h>
#include <chowdsp_gui/chowdsp_gui.h>

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

        juce::MessageManager::getInstance()->runDispatchLoopUntil (75);
        expectEquals (presetsComp.getPresetMenuText(), juce::String ("Test2"), "Initial preset text is incorrect!");

        setParameter (param, 0.9f);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (75);
        expectEquals (presetsComp.getPresetMenuText(), juce::String ("Test2*"), "Dirty preset text is incorrect!");

        presetsComp.goToNextPreset (true);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (75);
        expectEquals (presetsComp.getPresetMenuText(), juce::String ("Test1"), "Loaded preset text is incorrect!");

        setParameter (param, 0.9f);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (75);
        expectEquals (presetsComp.getPresetMenuText(), juce::String ("Test1*"), "Dirty preset text is incorrect!");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
    }

    static juce::PopupMenu::Item* getMenuItem (const juce::PopupMenu& menu, const juce::String& itemText)
    {
        auto menuIter = juce::PopupMenu::MenuItemIterator (menu, true);
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
        auto menuItem = getMenuItem (*menu, "Choose Preset Folder...");
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
        auto menuItem = getMenuItem (*menu, "Go to Preset Folder...");
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
        auto menuItem = getMenuItem (*menu, "Save Preset As");

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

        juce::MessageManager::getInstance()->runDispatchLoopUntil (75);
        expectEquals (presetsComp.getPresetMenuText(), juce::String ("Test2"), "Initial preset text is incorrect!");

        const auto* menu = presetsComp.getPresetMenuBox().getRootMenu();
        auto menuItem = getMenuItem (*menu, "Test1");
        menuItem->action();
        juce::MessageManager::getInstance()->runDispatchLoopUntil (75);
        expectEquals (presetsComp.getPresetMenuText(), juce::String ("Test1"), "Loaded preset text is incorrect!");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
    }

    void resetPresetTest()
    {
        DummyPlugin plugin { true };
        auto& presetMgr = plugin.getPresetManager();
        auto param = plugin.getParameters()[0];

        ScopedFile presetPath ("preset_path");
        presetPath.file.createDirectory();

        presetMgr.setUserPresetConfigFile ("preset_config.txt");
        presetMgr.setUserPresetPath (presetPath.file);

        ScopedFile presetFile1 ("preset_path/Test1.preset");
        presetMgr.saveUserPreset (presetFile1.file);
        const auto initialValue = param->getValue();
        setParameter (param, initialValue - 0.1f);

        chowdsp::PresetsComp presetsComp { presetMgr };
        const auto* menu = presetsComp.getPresetMenuBox().getRootMenu();
        auto resetItem = getMenuItem (*menu, "Reset");
        resetItem->action();

        expectEquals (param->getValue(), initialValue, "Reset did not correctly reset the parameter value");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
    }

    void resavePresetTest()
    {
        DummyPlugin plugin { true };
        auto& presetMgr = plugin.getPresetManager();
        auto param = plugin.getParameters()[0];

        ScopedFile presetPath ("preset_path");
        presetPath.file.createDirectory();
        presetMgr.setUserPresetConfigFile ("preset_config.txt");
        presetMgr.setUserPresetPath (presetPath.file);

        ScopedFile presetFile1 ("preset_path/Test1.preset");
        presetMgr.saveUserPreset (presetFile1.file);
        const auto initialValue = param->getValue();
        const auto secondaryValue = initialValue - 0.1f;
        setParameter (param, secondaryValue);

        chowdsp::PresetsComp presetsComp { presetMgr };
        const auto* menu = presetsComp.getPresetMenuBox().getRootMenu();
        getMenuItem (*menu, "Resave Preset")->action();

        setParameter (param, initialValue + 0.1f);
        presetMgr.loadPreset (chowdsp::Preset { presetFile1.file });

        expectWithinAbsoluteError (param->getValue(), secondaryValue, 1.0e-3f, "Resave did not correctly save the same preset");

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
        juce::Thread::launch ([&]
                              {
                                  juce::Thread::sleep (75); // wait for message manager...
            expectEquals (presetsComp.getPresetMenuText(), juce::String ("Test2"), "Initial preset text is incorrect!");

            const auto* menu = presetsComp.getPresetMenuBox().getRootMenu();
            if (auto* menuItem = getMenuItem (*menu, "Test1"))
                menuItem->action();
            juce::Thread::sleep (75); // wait for message manager...
            expectEquals (presetsComp.getPresetMenuText(), juce::String ("Test1"), "Loaded preset text is incorrect!");

            threadFinished = true; });

        while (! threadFinished)
            juce::MessageManager::getInstance()->runDispatchLoopUntil (75);

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

        beginTest ("Reset Preset Test");
        resetPresetTest();

        beginTest ("Resave Preset Test");
        resavePresetTest();

#if ! JUCE_WINDOWS
        beginTest ("Background Thread Test");
        backgroundThreadTest();
#endif
    }
};

static PresetsCompTest presetsCompTest;
