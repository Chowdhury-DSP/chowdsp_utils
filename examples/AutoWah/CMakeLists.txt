setup_example_plugin(AutoWah Auw3)

target_link_libraries(AutoWah
    PRIVATE
        chowdsp_dsp_utils
)

target_sources(AutoWah
    PRIVATE
        AutoWahPlugin.cpp
)
