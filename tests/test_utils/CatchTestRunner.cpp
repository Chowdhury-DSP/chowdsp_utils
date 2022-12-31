#include <catch2/catch_session.hpp>
#if JUCE_MODULE_AVAILABLE_juce_events
#include <juce_events/juce_events.h>
#endif

int main (int argc, char* argv[])
{
    // your setup ...
#if JUCE_MODULE_AVAILABLE_juce_events
    juce::ScopedJuceInitialiser_GUI scopedJuce {};
#endif

    int result = Catch::Session().run (argc, argv);

    // your clean-up...

    return result;
}
