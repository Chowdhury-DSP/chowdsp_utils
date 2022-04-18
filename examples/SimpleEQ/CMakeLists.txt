message(STATUS "Configuring example: SimpleEQ")

setup_example_plugin(SimpleEQ linP)

target_sources(SimpleEQ
    PRIVATE
        PrototypeEQ.cpp
        SimpleEQPlugin.cpp
)
