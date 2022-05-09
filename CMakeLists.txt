cmake_minimum_required(VERSION 3.15)
project(chowdsp_utils VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
include(CTest)

option(CHOWDSP_ENABLE_TESTING "Enable testing for chowdsp modules" OFF)
option(CHOWDSP_ENABLE_BENCHMARKS "Enable benchmarks for chowdsp modules" OFF)
option(CHOWDSP_ENABLE_EXAMPLES "Enable examples for chowdsp modules" OFF)

if(NOT (CHOWDSP_ENABLE_TESTING OR CHOWDSP_ENABLE_BENCHMARKS OR CHOWDSP_ENABLE_EXAMPLES))
    juce_add_modules(
        ALIAS_NAMESPACE chowdsp

        modules/common/chowdsp_core
        modules/common/chowdsp_json
        modules/common/chowdsp_reflection

        modules/dsp/chowdsp_dsp_data_structures
        modules/dsp/chowdsp_dsp_utils
        modules/dsp/chowdsp_eq
        modules/dsp/chowdsp_filters
        modules/dsp/chowdsp_math
        modules/dsp/chowdsp_reverb
        modules/dsp/chowdsp_simd

        modules/plugin/chowdsp_parameters
        modules/plugin/chowdsp_plugin_base
        modules/plugin/chowdsp_plugin_utils
        modules/plugin/chowdsp_presets
        modules/plugin/chowdsp_version

        modules/gui/chowdsp_gui
        modules/gui/chowdsp_foleys

        modules/music/chowdsp_rhythm
    )
else()
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
    if(CHOWDSP_ENABLE_TESTING)
        add_subdirectory(tests)
    elseif(CHOWDSP_ENABLE_BENCHMARKS)
        add_subdirectory(bench)
    elseif(CHOWDSP_ENABLE_EXAMPLES)
        add_subdirectory(examples)
    endif()
endif()
