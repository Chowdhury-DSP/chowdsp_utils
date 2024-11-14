juce_add_modules(
    ALIAS_NAMESPACE chowdsp

    ${PROJECT_SOURCE_DIR}/modules/common/chowdsp_core
    ${PROJECT_SOURCE_DIR}/modules/common/chowdsp_data_structures
    ${PROJECT_SOURCE_DIR}/modules/common/chowdsp_json
    ${PROJECT_SOURCE_DIR}/modules/common/chowdsp_listeners
    ${PROJECT_SOURCE_DIR}/modules/common/chowdsp_reflection
    ${PROJECT_SOURCE_DIR}/modules/common/chowdsp_serialization
    ${PROJECT_SOURCE_DIR}/modules/common/chowdsp_logging
    ${PROJECT_SOURCE_DIR}/modules/common/chowdsp_units

    ${PROJECT_SOURCE_DIR}/modules/dsp/chowdsp_buffers
    ${PROJECT_SOURCE_DIR}/modules/dsp/chowdsp_compressor
    ${PROJECT_SOURCE_DIR}/modules/dsp/chowdsp_dsp_data_structures
    ${PROJECT_SOURCE_DIR}/modules/dsp/chowdsp_dsp_utils
    ${PROJECT_SOURCE_DIR}/modules/dsp/chowdsp_eq
    ${PROJECT_SOURCE_DIR}/modules/dsp/chowdsp_math
    ${PROJECT_SOURCE_DIR}/modules/dsp/chowdsp_modal_dsp
    ${PROJECT_SOURCE_DIR}/modules/dsp/chowdsp_reverb
    ${PROJECT_SOURCE_DIR}/modules/dsp/chowdsp_simd
    ${PROJECT_SOURCE_DIR}/modules/dsp/chowdsp_sources
    ${PROJECT_SOURCE_DIR}/modules/dsp/chowdsp_filters
    ${PROJECT_SOURCE_DIR}/modules/dsp/chowdsp_waveshapers

    ${PROJECT_SOURCE_DIR}/modules/plugin/chowdsp_version
    ${PROJECT_SOURCE_DIR}/modules/plugin/chowdsp_presets
    ${PROJECT_SOURCE_DIR}/modules/plugin/chowdsp_presets_v2
    ${PROJECT_SOURCE_DIR}/modules/plugin/chowdsp_parameters
    ${PROJECT_SOURCE_DIR}/modules/plugin/chowdsp_plugin_base
    ${PROJECT_SOURCE_DIR}/modules/plugin/chowdsp_plugin_utils
    ${PROJECT_SOURCE_DIR}/modules/plugin/chowdsp_clap_extensions
    ${PROJECT_SOURCE_DIR}/modules/plugin/chowdsp_plugin_state
    ${PROJECT_SOURCE_DIR}/modules/plugin/chowdsp_fuzzy_search

    ${PROJECT_SOURCE_DIR}/modules/gui/chowdsp_gui
    ${PROJECT_SOURCE_DIR}/modules/gui/chowdsp_foleys
    ${PROJECT_SOURCE_DIR}/modules/gui/chowdsp_visualizers

    ${PROJECT_SOURCE_DIR}/modules/music/chowdsp_rhythm
)
