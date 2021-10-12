# chowdsp_utils

![CI](https://github.com/Chowdhury-DSP/chowdsp_utils/workflows/CI/badge.svg)
[![License](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://opensource.org/licenses/GPL-3.0)
[![codecov](https://codecov.io/gh/Chowdhury-DSP/chowdsp_utils/branch/master/graph/badge.svg?token=84B35MB5QS)](https://codecov.io/gh/Chowdhury-DSP/chowdsp_utils)

This repository contains a JUCE module with utilities for building Chowdhury DSP plugins.

The module currently contains the following utility classes:
- DSP
  - Delay Line: similar to `juce::dsp::DelayLine` but with 5th-order Lagrange interpolation and Sinc interpolation.
  - Pitch Shifter: using a ring buffer with two read pointers.
  - Filters: functions for bilinear transform, IIR filters with static order, and an optimized State Variable Filter.
  - Modal: modal filter/oscillator using Max Mathews Phasor Filter.
  - Sources: sine wave generator, noise generator
  - Wave Digital Filters: all linear circuit elements, series/parallel junctions, and diodes.
  - Some extra functions and classes for SIMD operations.
- GUI
  - Custom `LookAndFeel`.
  - Custom tooltip viewer.
  - Component to display plugin info (version, format, etc.).
- Plugin Utilities
  - Base class for audio effect plugin.
  - Base class for synth plugin.
  - Utility class for managing plugin presets.
  - Utility functions for creating parameter layouts.

For complete documentation, see the [API docs](https://ccrma.stanford.edu/~jatin/chowdsp/chowdsp_utils).

## License

`chowdsp_utils` is open source, and is licensed under the GPLv3.
Enjoy!
