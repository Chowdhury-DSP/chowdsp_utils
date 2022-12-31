setup_catch_juce_test(chowdsp_serialization_test juce::juce_gui_basics chowdsp::chowdsp_serialization)

target_sources(chowdsp_serialization_test PRIVATE
    SerializationTest.cpp
    TestSerialBinaryData.cpp
)
