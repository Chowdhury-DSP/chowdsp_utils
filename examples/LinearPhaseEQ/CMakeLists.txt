message(STATUS "Configuring example: LinearPhaseEQ")

setup_example_plugin(LinearPhaseEQ linP)

target_sources(LinearPhaseEQ
    PRIVATE
        PrototypeEQ.cpp
        LinearPhaseEQPlugin.cpp
)
