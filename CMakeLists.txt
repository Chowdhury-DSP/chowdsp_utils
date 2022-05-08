cmake_minimum_required(VERSION 3.15)
project(chowdsp_utils VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
include(CTest)

option(CHOWDSP_ENABLE_TESTING "Enable testing for chowdsp modules" OFF)
option(CHOWDSP_ENABLE_BENCHMARKS "Enable benchmarks for chowdsp modules" OFF)
option(CHOWDSP_ENABLE_EXAMPLES "Enable examples for chowdsp modules" OFF)

if(NOT (CHOWDSP_ENABLE_TESTING OR CHOWDSP_ENABLE_BENCHMARKS OR CHOWDSP_ENABLE_EXAMPLES))
    juce_add_modules(
        modules/common/chowdsp_core
        modules/common/chowdsp_json

        modules/chowdsp_dsp
        modules/chowdsp_gui
        modules/chowdsp_plugin_utils
    )
elseif(CHOWDSP_ENABLE_TESTING)
    add_subdirectory(tests)
elseif(CHOWDSP_ENABLE_BENCHMARKS)
    add_subdirectory(bench)
elseif(CHOWDSP_ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif()
