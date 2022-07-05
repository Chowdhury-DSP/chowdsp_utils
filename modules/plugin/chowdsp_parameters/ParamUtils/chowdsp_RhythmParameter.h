#pragma once

#include <chowdsp_rhythm/chowdsp_rhythm.h>

namespace chowdsp
{
/**
 * A parameter class derived from AudioParameterChoice representing
 * a rhythm. This might be useful for making a delay time synced to
 * the tempo of a song.
 */
class RhythmParameter : public juce::AudioParameterChoice
{
public:
    RhythmParameter (const ParameterID& paramID,
                     const juce::String& paramName,
                     const std::vector<RhythmUtils::Rhythm>& rhythms = RhythmUtils::getDefaultRhythms(),
                     const RhythmUtils::Rhythm& defaultRhythm = RhythmUtils::QUARTER);

    /** Returns the length of time associated with the current rhythm for a given BPM in seconds */
    double getRhythmTimeSeconds (double tempoBPM) const;

private:
    static juce::StringArray makeParameterChoiceList (const std::vector<RhythmUtils::Rhythm>& rhythms);
    static int getDefaultParameterChoice (const std::vector<RhythmUtils::Rhythm>& rhythms, const RhythmUtils::Rhythm& defaultRhythm);

    const std::vector<RhythmUtils::Rhythm> rhythmChoices;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhythmParameter)
};
} // namespace chowdsp
