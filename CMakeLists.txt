cmake_minimum_required(VERSION 3.15)
project(chowdsp_utils VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
include(CTest)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../JUCE build/JUCE)
juce_add_modules(../foleys_gui_magic)
juce_add_modules(.)

juce_add_console_app(chowdsp_utils_tests)
juce_generate_juce_header(chowdsp_utils_tests)
add_subdirectory(tests)

target_compile_definitions(chowdsp_utils_tests PRIVATE
    CHOWDSP_UNIT_TESTS=1
    JucePlugin_Name="DummyPlugin"
    JUCE_UNIT_TESTS=0
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0)

target_link_libraries(chowdsp_utils_tests PRIVATE
    chowdsp_utils
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

add_test(NAME chowdsp_utils_tests COMMAND chowdsp_utils_tests)
