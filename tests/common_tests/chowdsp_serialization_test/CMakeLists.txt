add_executable(chowdsp_serialization_test)
setup_juce_test(chowdsp_serialization_test)

target_link_libraries(chowdsp_serialization_test PRIVATE
    chowdsp_serialization
)

target_sources(chowdsp_serialization_test PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/test_utils/TestRunner.cpp
    SerializationTest.cpp
)