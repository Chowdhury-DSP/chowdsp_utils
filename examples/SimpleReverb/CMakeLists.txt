setup_example_plugin(SimpleReverb sRvb)

target_link_libraries(SimpleReverb
    PRIVATE
        chowdsp_reverb
)

target_sources(SimpleReverb
    PRIVATE
        SimpleReverbPlugin.cpp
)
