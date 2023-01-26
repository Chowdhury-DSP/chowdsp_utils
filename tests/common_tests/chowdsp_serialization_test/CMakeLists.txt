setup_catch_lib_test(chowdsp_serialization_test common_tests_lib)

target_sources(chowdsp_serialization_test PRIVATE
    SerializationTest.cpp
    TestSerialBinaryData.cpp
)
