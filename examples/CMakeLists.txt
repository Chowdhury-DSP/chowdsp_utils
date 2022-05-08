if("${CHOWDSP_PATH_TO_JUCE}" STREQUAL "")
    message(STATUS "No JUCE path provided! Assuming JUCE has already been added.")
else()
    message(STATUS "Loading JUCE from: ${CHOWDSP_PATH_TO_JUCE}")
    add_subdirectory(${CMAKE_SOURCE_DIR}/${CHOWDSP_PATH_TO_JUCE} JUCE)
endif()

include(AddJUCEModules)
include(SetupExamplePlugin)

add_compile_options(
  $<$<CXX_COMPILER_ID:MSVC>:/WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
)

add_subdirectory(SimpleEQ)
add_subdirectory(SimpleReverb)
add_subdirectory(SignalGenerator)
add_subdirectory(ModalSpringReverb)
add_subdirectory(AutoWah)
