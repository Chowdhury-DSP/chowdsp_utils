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
  GITHUB_REPOSITORY jatinchowdhury18/clap-juce-extensions
  GIT_TAG c8daec07377050b75c9813dc71a85f4d3b669a93
)

include(AddJUCEModules)
include(SetupExamplePlugin)

# Turning this off until clap-juce-extensions
# gets a little more stable
#add_compile_options(
#  $<$<CXX_COMPILER_ID:MSVC>:/WX>
#  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
#)

add_subdirectory(SimpleEQ)
add_subdirectory(SimpleReverb)
add_subdirectory(SignalGenerator)
add_subdirectory(ModalSpringReverb)
add_subdirectory(AutoWah)
add_subdirectory(AccessiblePlugin)
add_subdirectory(ForwardingTestPlugin)
