include(CPM)

# download JUCE
CPMAddPackage(
  NAME juce
  GITHUB_REPOSITORY juce-framework/juce
  GIT_TAG 6.1.5
)

# download CLAP extensions
CPMAddPackage(
  NAME clap-juce-extensions
  GITHUB_REPOSITORY free-audio/clap-juce-extensions
  GIT_TAG 326c380d1f737ac255a377e5ad7b300b61864b3f
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
