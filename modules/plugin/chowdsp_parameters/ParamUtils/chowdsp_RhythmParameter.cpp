#include "chowdsp_RhythmParameter.h"

namespace chowdsp
{
RhythmParameter::RhythmParameter (const ParameterID& parameterID,
                                  const juce::String& paramName,
                                  const std::vector<RhythmUtils::Rhythm>& rhythms,
                                  const RhythmUtils::Rhythm& defaultRhythm) : chowdsp::ChoiceParameter (parameterID,
                                                                                                        paramName,
                                                                                                        makeParameterChoiceList (rhythms),
                                                                                                        getDefaultParameterChoice (rhythms, defaultRhythm)),
                                                                              rhythmChoices (rhythms)
{
}

RhythmUtils::Rhythm RhythmParameter::getRhythm() const
{
    const auto currentRhythmIndex = getIndex();
    return rhythmChoices[(size_t) currentRhythmIndex];
}

double RhythmParameter::getRhythmTimeSeconds (double tempoBPM) const
{
    return getRhythm().getTimeSeconds (tempoBPM);
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
