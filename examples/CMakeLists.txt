include(CPM)

# download JUCE
CPMAddPackage(
  NAME juce
  GITHUB_REPOSITORY juce-framework/juce
  GIT_TAG 7.0.6
)

# download CLAP extensions
CPMAddPackage(
  NAME clap-juce-extensions
  GITHUB_REPOSITORY free-audio/clap-juce-extensions
  GIT_TAG e72d59a870ab6dea156d4912cbd004b715fca5f7
)

include(AddJUCEModules)
include(SetupExamplePlugin)

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
