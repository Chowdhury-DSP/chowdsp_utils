setup_example_plugin(AutoWah Auw3)

target_link_libraries(AutoWah
    PRIVATE
        juce::juce_dsp
        chowdsp::chowdsp_dsp_utils
)

target_sources(AutoWah
    PRIVATE
        AutoWahPlugin.cpp
)
