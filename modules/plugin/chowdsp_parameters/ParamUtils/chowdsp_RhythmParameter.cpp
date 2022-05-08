#include "chowdsp_RhythmParameter.h"

namespace chowdsp
{
RhythmParameter::RhythmParameter (const juce::String& parameterID,
                                  const juce::String& paramName,
                                  const std::vector<RhythmUtils::Rhythm>& rhythms,
                                  const RhythmUtils::Rhythm& defaultRhythm) : juce::AudioParameterChoice (parameterID,
                                                                                                          paramName,
                                                                                                          makeParameterChoiceList (rhythms),
                                                                                                          getDefaultParameterChoice (rhythms, defaultRhythm)),
                                                                              rhythmChoices (rhythms)
{
}

double RhythmParameter::getRhythmTimeSeconds (double tempoBPM) const
{
    const auto currentRhythmIndex = getIndex();
    return rhythmChoices[(size_t) currentRhythmIndex].getTimeSeconds (tempoBPM);
}

juce::StringArray RhythmParameter::makeParameterChoiceList (const std::vector<RhythmUtils::Rhythm>& rhythms)
{
    juce::StringArray choices;
    for (const auto& r : rhythms)
        choices.add (r.getLabel());

    return choices;
}

int RhythmParameter::getDefaultParameterChoice (const std::vector<RhythmUtils::Rhythm>& rhythms, const RhythmUtils::Rhythm& defaultRhythm)
{
    int i = 0;
    for (const auto& r : rhythms)
    {
        if (r == defaultRhythm)
            return i;

        i++;
    }

    jassertfalse; // default rhythm was not found in list!
    return 0;
}
} // namespace chowdsp
