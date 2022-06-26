add_executable(chowdsp_eq_test)
setup_juce_test(chowdsp_eq_test)

target_link_libraries(chowdsp_eq_test PRIVATE
    chowdsp_eq
)

target_sources(chowdsp_eq_test PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/test_utils/TestRunner.cpp
    LinearPhaseEQTest.cpp
)
