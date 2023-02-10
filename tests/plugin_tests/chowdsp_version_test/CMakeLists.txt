setup_catch_lib_test(chowdsp_version_test plugin_tests_lib)

target_sources(chowdsp_version_test
    PRIVATE
        VersionUtilsTest.cpp
)
