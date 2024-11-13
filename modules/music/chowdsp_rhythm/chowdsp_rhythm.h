/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_rhythm
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Rhythm Utilities
   description:   Utility classes for working with rhythms
   dependencies:  chowdsp_core

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <initializer_list>
#include <string_view>
#include <vector>
#include <chowdsp_core/chowdsp_core.h>

namespace chowdsp
{
/** Utilities for managing rhythms in code */
namespace RhythmUtils
{
    /** Simple struct representing the rhythm of a note */
    struct Rhythm
    {
        constexpr Rhythm (const std::string_view& rhythmName,
                          const std::string_view& rhythmLabel,
                          double rhythmTempoFactor) : name (rhythmName),
                                                      label (rhythmLabel),
                                                      tempoFactor (rhythmTempoFactor) {}

        /** Returns the rhythm's label (i.e. a quarter note has the label "1/4" */
        [[nodiscard]] inline std::string getLabel() const { return std::string (label); }

        /** Return time in seconds for rhythm and tempo */
        [[nodiscard]] constexpr double getTimeSeconds (double tempoBPM) const
        {
            const auto beatLength = 1.0 / (tempoBPM / 60.0);
            return beatLength * tempoFactor;
        }

        constexpr bool operator== (const Rhythm& other) const
        {
            return juce::exactlyEqual (tempoFactor, other.tempoFactor);
        }

        constexpr bool operator!= (const Rhythm& other) const
        {
            return ! (*this == other);
        }

        std::string_view name;
        std::string_view label;
        double tempoFactor;
    };

    constexpr auto THIRTY_SECOND = Rhythm ("Thirty-Second", "1/32", 0.125);
    constexpr auto THIRTY_SECOND_DOT = Rhythm ("Thirty-Second Dot", "1/32 D", 0.125 * 1.5);
    constexpr auto SIXTEENTH_TRIPLET = Rhythm ("Sixteenth Triplet", "1/16 T", 0.5 / 3.0);
    constexpr auto SIXTEENTH = Rhythm ("Sixteenth", "1/16", 0.25);
    constexpr auto SIXTEENTH_DOT = Rhythm ("Sixteenth Dot", "1/16 D", 0.25 * 1.5);
    constexpr auto EIGHTH_TRIPLET = Rhythm ("Eighth Triplet", "1/8 T", 1.0 / 3.0);
    constexpr auto EIGHTH = Rhythm ("Eighth", "1/8", 0.5);
    constexpr auto EIGHTH_DOT = Rhythm ("Eighth Dot", "1/8 D", 0.5 * 1.5);
    constexpr auto QUARTER_TRIPLET = Rhythm ("Quarter Triplet", "1/4 T", 2.0 / 3.0);
    constexpr auto QUARTER = Rhythm ("Quarter", "1/4", 1.0);
    constexpr auto QUARTER_DOT = Rhythm ("Quarter Dot", "1/4 D", 1.0 * 1.5);
    constexpr auto HALF_TRIPLET = Rhythm ("Half Triplet", "1/2 T", 4.0 / 3.0);
    constexpr auto HALF = Rhythm ("Half", "1/2", 2.0);
    constexpr auto HALF_DOT = Rhythm ("Half Dot", "1/2 D", 2.0 * 1.5);
    constexpr auto WHOLE_TRIPLET = Rhythm ("Whole Triplet", "1/1 T", 8.0 / 3.0);
    constexpr auto WHOLE = Rhythm ("Whole", "1/1", 4.0);
    constexpr auto WHOLE_DOT = Rhythm ("Whole Dot", "1/1 D", 4.0 * 1.5);
    constexpr auto TWO_WHOLE_TRIPLET = Rhythm ("Two Whole Triplet", "2/1 T", 16.0 / 3.0);
    constexpr auto TWO_WHOLE = Rhythm ("Two Whole", "2/1", 8.0);

    /** Returns a list of basic default rhythms */
    [[maybe_unused]] inline auto getDefaultRhythms()
    {
        return std::vector<Rhythm> {
            SIXTEENTH,
            SIXTEENTH_DOT,
            EIGHTH_TRIPLET,
            EIGHTH,
            EIGHTH_DOT,
            QUARTER_TRIPLET,
            QUARTER,
            QUARTER_DOT,
            HALF_TRIPLET,
            HALF,
            HALF_DOT,
            WHOLE_TRIPLET,
            WHOLE,
        };
    }
} // namespace RhythmUtils
} // namespace chowdsp
