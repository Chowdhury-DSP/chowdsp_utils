include(SubprojectVersion)
include(EnableCoverageFlags)
include(SetupJuceTest)
include(SetupCatchTest)
include(SetupChowdspLib)

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
juce_add_module(${CMAKE_SOURCE_DIR}/../foleys_gui_magic)
add_subdirectory(${CMAKE_SOURCE_DIR}/../clap-juce-extensions clap_juce_extensions EXCLUDE_FROM_ALL)

option(CODE_COVERAGE "Enable coverage reporting" OFF)

add_subdirectory(common_tests)
add_subdirectory(dsp_tests)
add_subdirectory(plugin_tests)
add_subdirectory(gui_tests)
add_subdirectory(music_tests)

option(CHOWDSP_CODE_QUALITY_CHECKS "Configure code quality checks for chowdsp modules" OFF)
if(CHOWDSP_CODE_QUALITY_CHECKS)
    include(SetupCodeQuality)
endif()

option(CHOWDSP_ENABLE_STATIC_TESTS "Configure compile-time static tests for chowdsp modules" OFF)
if(CHOWDSP_ENABLE_STATIC_TESTS)
    add_subdirectory(static_tests)
endif()
