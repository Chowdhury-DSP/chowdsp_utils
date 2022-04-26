#pragma once

#include "../MusicTheory/chowdsp_RhythmUtils.h"

namespace chowdsp
{

class RhythmParameter : public juce::AudioParameterChoice
{
public:
    RhythmParameter (const juce::String& paramID,
                     const juce::String& paramName,
                     const std::vector<RhythmUtils::Rhythm>& rhythms = RhythmUtils::getDefaultRhythms(),
                     const RhythmUtils::Rhythm& defaultRhythm = RhythmUtils::QUARTER);

    double getRhythmTimeSeconds (double tempoBPM) const;

private:
    static juce::StringArray makeParameterChoiceList (const std::vector<RhythmUtils::Rhythm>& rhythms);
    static int getDefaultParameterChoice (const std::vector<RhythmUtils::Rhythm>& rhythms, const RhythmUtils::Rhythm& defaultRhythm);

    const std::vector<RhythmUtils::Rhythm> rhythmChoices;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhythmParameter)
};
} // namespace chowdsp
