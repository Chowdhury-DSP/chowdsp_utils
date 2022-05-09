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
#add_subdirectory(dsp_tests)
#add_subdirectory(plugin_tests)
add_subdirectory(gui_tests)
#add_subdirectory(music_tests)

option(CHOWDSP_RUN_CLANG_TIDY "Run clang-tidy on chowdsp modules" OFF)
if(CHOWDSP_RUN_CLANG_TIDY)
    message(STATUS "Configuring clang-tidy target")
    add_custom_target(chowdsp_utils_clang_tidy
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND clang-tidy -p build "modules/dsp/chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.cpp"
        COMMAND clang-tidy -p build "modules/dsp/chowdsp_dsp_utils/chowdsp_dsp_utils.cpp"
        COMMAND clang-tidy -p build "modules/dsp/chowdsp_math/chowdsp_math.cpp"
        COMMAND clang-tidy -p build "modules/gui/chowdsp_gui/chowdsp_gui.cpp"
        COMMAND clang-tidy -p build "modules/plugin/chowdsp_parameters/chowdsp_parameters.cpp"
        COMMAND clang-tidy -p build "modules/plugin/chowdsp_plugin_utils/chowdsp_plugin_utils.cpp"
        COMMAND clang-tidy -p build "modules/plugin/chowdsp_presets/chowdsp_presets.cpp"
        COMMAND clang-tidy -p build "modules/plugin/chowdsp_version/chowdsp_version.cpp"
    )
endif()
