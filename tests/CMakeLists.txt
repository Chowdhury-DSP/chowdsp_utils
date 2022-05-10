include(SubprojectVersion)
include(EnableCoverageFlags)
include(SetupJuceTest)

message(STATUS "Configuring tests for chowdsp_utils")

# set up JUCE
option(CHOWDSP_BUILD_LIVE_GUI_TEST "Build live GUI test app" OFF)
if(NOT CHOWDSP_BUILD_LIVE_GUI_TEST)
    message(STATUS "Live GUI test is not being built... skipping juceaide!")
    set(JUCE_MODULES_ONLY ON CACHE INTERNAL "Only configure the JUCE modules")
endif()
add_subdirectory(${CMAKE_SOURCE_DIR}/../../JUCE JUCE)
subproject_version(JUCE juce_version)
message(STATUS "VERSION for JUCE: ${juce_version}")

include(AddJUCEModules)

option(CODE_COVERAGE "Enable coverage reporting" OFF)

add_subdirectory(common_tests)
add_subdirectory(dsp_tests)
add_subdirectory(plugin_tests)
add_subdirectory(gui_tests)
add_subdirectory(music_tests)

option(CHOWDSP_CODE_QUALITY_CHECKS "Configure code quality checks for chowdsp modules" OFF)
if(CHOWDSP_CODE_QUALITY_CHECKS)
    message(STATUS "Configuring clang-tidy target...")
    add_custom_target(chowdsp_utils_clang_tidy
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND clang-tidy -p build "tests/test_utils/CodeQuality.cpp"
    )

    message(STATUS "Configuring CodeQL target...")
    add_library(chowdsp_utils_codeql)
    target_sources(chowdsp_utils_codeql PRIVATE test_utils/CodeQuality.cpp)
    target_compile_definitions(chowdsp_utils_codeql PRIVATE
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_MODAL_LOOPS_PERMITTED=1
        JUCE_STANDALONE_APPLICATION=0
    )

    target_link_libraries(chowdsp_utils_codeql PRIVATE
        juce::juce_dsp
        chowdsp_core
        chowdsp_json
        chowdsp_reflection
        chowdsp_dsp_data_structures
        chowdsp_dsp_utils
        chowdsp_eq
        chowdsp_filters
        chowdsp_math
        chowdsp_reverb
        chowdsp_simd
        chowdsp_gui
        chowdsp_rhythm
        chowdsp_parameters
        chowdsp_plugin_base
        chowdsp_plugins_utils
        chowdsp_presets
        chowdsp_version
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    )
endif()
