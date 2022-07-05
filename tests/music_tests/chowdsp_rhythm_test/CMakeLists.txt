add_executable(chowdsp_rhythm_test)
setup_juce_test(chowdsp_rhythm_test)

target_link_libraries(chowdsp_rhythm_test PRIVATE
    chowdsp_rhythm
)

target_sources(chowdsp_rhythm_test PRIVATE
    RhythmUtilsTest.cpp
)
