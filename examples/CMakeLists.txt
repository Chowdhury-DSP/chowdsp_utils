include(CPM)

# download JUCE
CPMAddPackage(
  NAME juce
  GITHUB_REPOSITORY juce-framework/juce
  GIT_TAG 8.0.10
)

include(AddJUCEModules)

# OSX deployment target must be defined at a project level
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment target" FORCE)
project(chowdsp_utils_examples VERSION 2.4.0)

# download CLAP extensions
CPMAddPackage(
    NAME clap-juce-extensions
    GITHUB_REPOSITORY free-audio/clap-juce-extensions
    GIT_TAG 4d454e5125da75a0e75d95615cbec26d2a09e2bf
)

include(SetupExamplePlugin)

# Treat warnings as errors
add_compile_options(
  $<$<CXX_COMPILER_ID:MSVC>:/WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
)

add_subdirectory(SimpleEQ)
add_subdirectory(SimpleReverb)
add_subdirectory(SignalGenerator)
add_subdirectory(ModalSpringReverb)
add_subdirectory(AutoWah)
add_subdirectory(AccessiblePlugin)
add_subdirectory(ForwardingTestPlugin)
add_subdirectory(StatefulPlugin)
add_subdirectory(ExampleCompressor)
