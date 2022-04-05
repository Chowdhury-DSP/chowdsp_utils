message(STATUS "Configuring example: Signal Generator")

juce_add_plugin(SignalGenerator
    COMPANY_NAME chowdsp
    PLUGIN_MANUFACTURER_CODE Chow
    PLUGIN_CODE Sggb
    FORMATS VST3 AU Standalone
    PRODUCT_NAME SignalGenerator

    MICROPHONE_PERMISSION_ENABLED TRUE
)

target_sources(SignalGenerator
    PRIVATE
        SignalGeneratorPlugin.cpp
)

target_compile_definitions(SignalGenerator
    PUBLIC
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(SignalGenerator
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        chowdsp_dsp
        chowdsp_plugin_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
