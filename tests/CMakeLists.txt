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
    ${CMAKE_SOURCE_DIR}/modules/chowdsp_dsp
    ${CMAKE_SOURCE_DIR}/modules/chowdsp_gui
    ${CMAKE_SOURCE_DIR}/modules/chowdsp_plugin_utils)

option(CODE_COVERAGE "Enable coverage reporting" OFF)

add_subdirectory(chowdsp_utils_test)
add_subdirectory(chowdsp_gui_test)
add_subdirectory(wdf_standalone_test)
