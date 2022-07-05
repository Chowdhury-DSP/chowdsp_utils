add_executable(chowdsp_serialization_test)
setup_juce_test(chowdsp_serialization_test)

target_link_libraries(chowdsp_serialization_test PRIVATE
    juce_gui_basics
    chowdsp_serialization
)

target_sources(chowdsp_serialization_test PRIVATE
    SerializationTest.cpp

    TestSerialBinaryData.cpp
)
