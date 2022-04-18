if("${CHOWDSP_PATH_TO_JUCE}" STREQUAL "")
    message(STATUS "No JUCE path provided! Assuming JUCE has already been added.")
else()
    message(STATUS "Loading JUCE from: ${CHOWDSP_PATH_TO_JUCE}")
    add_subdirectory(${CMAKE_SOURCE_DIR}/${CHOWDSP_PATH_TO_JUCE} JUCE)
endif()

juce_add_modules(
    ${CMAKE_SOURCE_DIR}/modules/chowdsp_dsp
    ${CMAKE_SOURCE_DIR}/modules/chowdsp_gui
    ${CMAKE_SOURCE_DIR}/modules/chowdsp_plugin_utils
)

add_compile_options(
  $<$<CXX_COMPILER_ID:MSVC>:/WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
)

function(setup_example_plugin target code)
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
            juce::juce_dsp
            chowdsp_dsp
            chowdsp_plugin_utils
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )
endfunction(setup_example_plugin)

add_subdirectory(SimpleEQ)
add_subdirectory(SimpleReverb)
add_subdirectory(SignalGenerator)
add_subdirectory(ModalSpringReverb)
