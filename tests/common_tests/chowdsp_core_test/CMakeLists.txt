add_executable(chowdsp_core_test)
setup_juce_test(chowdsp_core_test)

target_link_libraries(chowdsp_core_test PRIVATE
    chowdsp_core
)

target_sources(chowdsp_core_test PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/test_utils/TestRunner.cpp

    DoubleBufferTest.cpp
    AtomicHelpersTest.cpp
    MemoryAliasingTest.cpp
)
