# chowdsp_utils

[![Build Status](https://travis-ci.com/Chowdhury-DSP/chowdsp_utils.svg?token=Ub9niJrqG1Br1qaaxp7E&branch=master)](https://travis-ci.com/Chowdhury-DSP/chowdsp_utils)
[![License](https://img.shields.io/badge/License-BSD-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

This repository contains a JUCE module with utilities for building Chowdhury DSP plugins.

The module currently contains the following utility classes:
- DSP
  - Delay Line: similar to `juce::dsp::DelayLine` but with 5th-order Lagrange interpolation.
  - Filters: functions for bilinear transform, IIR filters with static order.
  - Modal: modal filter/oscillator using Max Mathews Phasor Filter.
  - Sources: sine wave generator, noise generator
  - Wave Digital Filters: all linear circuit elements, series/parallel junctions, and diodes.
- GUI
  - Custom `LookAndFeel`.
  - Custom tooltip viewer.
  - Component to display plugin info (version, format, etc.).
- Plugin Utilities
  - Base class for audio effect plugin.
  - Base class for synth plugin.

For complete documentation, see the [API docs](https://ccrma.stanford.edu/~jatin/chowdsp/chowdsp_utils/docs/).

## License

`chowdsp_utils` is open source, and is licensed under the BSD 3-clause license.
Enjoy!
