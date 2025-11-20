setup_catch_lib_test(chowdsp_parameters_test plugin_tests_lib)

target_sources(chowdsp_parameters_test PRIVATE
    ForwardingParameterTest.cpp
    ParamHelpersTest.cpp
    ParamModulationTest.cpp
    ParamStringsTest.cpp
    BoolParameterTest.cpp
    RhythmParameterTest.cpp
    MetricParameterTest.cpp
)
