setup_catch_lib_test(chowdsp_logging_test common_tests_lib)

target_sources(chowdsp_logging_test PRIVATE
    PluginLoggerTest.cpp
    CustomFormattingTest.cpp
)
