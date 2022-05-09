add_executable(chowdsp_version_test)
setup_juce_test(chowdsp_version_test)

target_link_libraries(chowdsp_version_test PRIVATE
    chowdsp_version
)

target_sources(chowdsp_version_test PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/test_utils/TestRunner.cpp
    VersionUtilsTest.cpp
)
