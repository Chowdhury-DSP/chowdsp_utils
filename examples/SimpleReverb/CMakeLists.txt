message(STATUS "Configuring example: SimpleReverb")

juce_add_plugin(SimpleReverb
    COMPANY_NAME chowdsp
    PLUGIN_MANUFACTURER_CODE Chow
    PLUGIN_CODE sRvb
    FORMATS VST3 AU Standalone
    PRODUCT_NAME SimpleReverb

    MICROPHONE_PERMISSION_ENABLED TRUE
)

target_sources(SimpleReverb
    PRIVATE
        SimpleReverbPlugin.cpp
)

target_compile_definitions(SimpleReverb
    PUBLIC
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(SimpleReverb
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
