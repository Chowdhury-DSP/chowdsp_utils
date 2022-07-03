add_executable(chowdsp_json_test)
setup_juce_test(chowdsp_json_test)

target_link_libraries(chowdsp_json_test PRIVATE
    chowdsp_json
)

target_sources(chowdsp_json_test PRIVATE
    JSONTest.cpp
)
