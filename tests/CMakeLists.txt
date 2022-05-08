list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(SubprojectVersion)
include(EnableCoverageFlags)
include(SetupJuceTest)

message(STATUS "Configuring tests for chowdsp_utils")

# set up JUCE
add_subdirectory(${CMAKE_SOURCE_DIR}/../../JUCE JUCE)
subproject_version(JUCE juce_version)
message(STATUS "VERSION for JUCE: ${juce_version}")

juce_add_modules(
    ${CMAKE_SOURCE_DIR}/../foleys_gui_magic
    ${CMAKE_SOURCE_DIR}/modules/common/chowdsp_core
    ${CMAKE_SOURCE_DIR}/modules/common/chowdsp_json
    ${CMAKE_SOURCE_DIR}/modules/dsp/chowdsp_dsp_data_structures
    ${CMAKE_SOURCE_DIR}/modules/dsp/chowdsp_dsp_utils
    ${CMAKE_SOURCE_DIR}/modules/dsp/chowdsp_eq
    ${CMAKE_SOURCE_DIR}/modules/dsp/chowdsp_math
    ${CMAKE_SOURCE_DIR}/modules/dsp/chowdsp_reverb
    ${CMAKE_SOURCE_DIR}/modules/dsp/chowdsp_simd
    ${CMAKE_SOURCE_DIR}/modules/dsp/chowdsp_filters
    ${CMAKE_SOURCE_DIR}/modules/plugin/chowdsp_version
    ${CMAKE_SOURCE_DIR}/modules/plugin/chowdsp_presets
    ${CMAKE_SOURCE_DIR}/modules/plugin/chowdsp_parameters
    ${CMAKE_SOURCE_DIR}/modules/plugin/chowdsp_plugin_base
    ${CMAKE_SOURCE_DIR}/modules/plugin/chowdsp_plugin_utils
    ${CMAKE_SOURCE_DIR}/modules/gui/chowdsp_gui
    ${CMAKE_SOURCE_DIR}/modules/gui/chowdsp_foleys
    ${CMAKE_SOURCE_DIR}/modules/music/chowdsp_rhythm
)

option(CODE_COVERAGE "Enable coverage reporting" OFF)

add_subdirectory(common_tests)
add_subdirectory(dsp_tests)
add_subdirectory(plugin_tests)
add_subdirectory(gui_tests)
add_subdirectory(music_tests)

option(CHOWDSP_RUN_CLANG_TIDY "Run clang-tidy on chowdsp modules" OFF)
if(CHOWDSP_RUN_CLANG_TIDY)
    message(STATUS "Configuring clang-tidy target")
    add_custom_target(chowdsp_utils_clang_tidy
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND clang-tidy -p build "modules/chowdsp_dsp/chowdsp_dsp.cpp" "modules/chowdsp_gui/chowdsp_gui.cpp" "modules/chowdsp_plugin_utils/chowdsp_plugin_utils.cpp"
    )
endif()
