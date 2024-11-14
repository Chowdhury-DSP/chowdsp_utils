/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_visualizers
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Visualizer Utilities
   description:   UI visualizer components for ChowDSP plugins
   dependencies:  juce_gui_basics, juce_audio_basics, chowdsp_filters

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       GPLv3

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <complex>

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <chowdsp_filters/chowdsp_filters.h>

#if JUCE_MODULE_AVAILABLE_chowdsp_eq
#include <chowdsp_eq/chowdsp_eq.h>
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>
#endif

#include "SpectrumPlots/chowdsp_SpectrumPlotBase.h"
#include "SpectrumPlots/chowdsp_EQFilterPlots.h"
#include "SpectrumPlots/chowdsp_EqualizerPlot.h"
#include "SpectrumPlots/chowdsp_GenericFilterPlotter.h"

#include "TimeDomain/chowdsp_WaveformView.h"

#include "WaveshaperPlot/chowdsp_WaveshaperPlot.h"

#include "CompressorPlots/chowdsp_GainReductionMeter.h"
#include "CompressorPlots/chowdsp_GainComputerPlot.h"
#include "CompressorPlots/chowdsp_LevelDetectorVisualizer.h"
