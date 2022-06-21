# setup_example_plugin(<target-name> <plugin-code>)
#
# Sets up an example JUCE plugin
function(setup_example_plugin target code)
    message(STATUS "Configuring example: ${target}")

    juce_add_plugin(${target}
        COMPANY_NAME chowdsp
        PLUGIN_MANUFACTURER_CODE Chow
        PLUGIN_CODE ${code}
        FORMATS VST3 AU Standalone
        PRODUCT_NAME ${target}
        MICROPHONE_PERMISSION_ENABLED TRUE
    )

    target_compile_definitions(${target}
        PUBLIC
            JUCE_DISPLAY_SPLASH_SCREEN=0
            JUCE_REPORT_APP_USAGE=0
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_VST3_CAN_REPLACE_VST2=0
    )

    target_link_libraries(${target}
        PRIVATE
            juce::juce_audio_utils
            chowdsp_plugin_base
            chowdsp_clap_extensions
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )

    clap_juce_extensions_plugin(
        TARGET ${target}
        CLAP_ID "org.chowdsp.${target}"
        CLAP_FEATURES audio-effect
    )
endfunction(setup_example_plugin)
