setup_catch_lib_test(chowdsp_plugin_base_test plugin_tests_lib)

target_sources(chowdsp_plugin_base_test PRIVATE
    PluginBaseTest.cpp
    PluginDiagnosticInfoTest.cpp
)

include(AddDiagnosticInfo)
add_diagnostic_info(chowdsp_plugin_base_test)
