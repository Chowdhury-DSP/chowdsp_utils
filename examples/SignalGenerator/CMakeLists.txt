message(STATUS "Configuring example: Signal Generator")

setup_example_plugin(SignalGenerator Sggb)

target_sources(SignalGenerator
    PRIVATE
        SignalGeneratorPlugin.cpp
)
