juce_add_gui_app(chowdsp_gui_test)
setup_juce_test(chowdsp_gui_test)

target_sources(chowdsp_gui_test PRIVATE GUITest.cpp)

# set coverage flags if needed
if(CODE_COVERAGE)
    enable_coverage_flags(chowdsp_gui_test)
endif()
