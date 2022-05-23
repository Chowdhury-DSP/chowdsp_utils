juce_add_modules(
    ${CMAKE_SOURCE_DIR}/modules/common/chowdsp_core
    ${CMAKE_SOURCE_DIR}/modules/common/chowdsp_json
    ${CMAKE_SOURCE_DIR}/modules/common/chowdsp_reflection
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
