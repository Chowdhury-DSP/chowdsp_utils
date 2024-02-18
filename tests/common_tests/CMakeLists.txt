setup_juce_lib(common_tests_lib 
    juce::juce_gui_basics
    chowdsp::chowdsp_core
    chowdsp::chowdsp_data_structures
    chowdsp::chowdsp_serialization
    chowdsp::chowdsp_logging
    chowdsp::chowdsp_json
    chowdsp::chowdsp_units
)

add_subdirectory(chowdsp_core_test)
add_subdirectory(chowdsp_data_structures_test)
add_subdirectory(chowdsp_json_test)
add_subdirectory(chowdsp_serialization_test)
add_subdirectory(chowdsp_logging_test)
add_subdirectory(chowdsp_units_test)
