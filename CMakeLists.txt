cmake_minimum_required(VERSION 3.15)
project(chowdsp_utils VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)

option(CHOWDSP_ENABLE_TESTING "Enable testing for chowdsp modules" OFF)

if(NOT CHOWDSP_ENABLE_TESTING)
    juce_add_modules(
        modules/chowdsp_dsp
        modules/chowdsp_gui
        modules/chowdsp_plugin_utils
    )
else()
    add_subdirectory(tests)
endif()
