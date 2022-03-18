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

add_subdirectory(LinearPhaseEQ)
