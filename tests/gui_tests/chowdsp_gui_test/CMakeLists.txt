setup_catch_lib_test(chowdsp_gui_test gui_tests_lib)

target_sources(chowdsp_gui_test PRIVATE
    LongPressActionTest.cpp
    OpenGLHelperTest.cpp
    PopupMenuHelperTest.cpp
    WindowInPluginTest.cpp
    HostContextProviderTest.cpp
    ComponentArenaTest.cpp
    ParametersViewTest.cpp
)
