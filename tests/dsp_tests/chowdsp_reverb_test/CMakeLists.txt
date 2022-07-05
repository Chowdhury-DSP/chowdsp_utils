add_executable(chowdsp_reverb_test)
setup_juce_test(chowdsp_reverb_test)

target_link_libraries(chowdsp_reverb_test PRIVATE
    chowdsp_reverb
)

target_sources(chowdsp_reverb_test PRIVATE
    DiffuserTest.cpp
)
