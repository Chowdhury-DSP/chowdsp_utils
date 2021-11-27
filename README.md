# chowdsp_utils

![CI](https://github.com/Chowdhury-DSP/chowdsp_utils/workflows/CI/badge.svg)
[![License](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://opensource.org/licenses/GPL-3.0)
[![codecov](https://codecov.io/gh/Chowdhury-DSP/chowdsp_utils/branch/master/graph/badge.svg?token=84B35MB5QS)](https://codecov.io/gh/Chowdhury-DSP/chowdsp_utils)

This repository contains JUCE modules with utilities for building Chowdhury DSP plugins.

There are currently 3 modules each containing the following classes:
- chowdsp_dsp
  - Delay Line: similar to `juce::dsp::DelayLine` but with 5th-order Lagrange interpolation and Sinc interpolation.
  - Pitch Shifter: using a ring buffer with two read pointers.
  - Filters: functions for bilinear transform, IIR filters with static order, and an optimized State Variable Filter.
  - Modal: modal filter/oscillator using Max Mathews Phasor Filter.
  - Sources: "magic circle" sine wave oscillator, noise generator.
  - Resampling: classes for doing non-integer sample rate conversion, and processing audio at a target sample rate.
  - Wave Digital Filters: linear circuit elements, series/parallel junctions, diodes, and a few others.
  - Some extra functions and classes for SIMD operations.
- chowdsp_gui
  - Custom `LookAndFeel`.
  - Utility class for managing `LookAndFeel` allocation.
  - Custom tooltip viewer.
  - Component to display plugin info (version, format, etc.).
  - Component for controlling preset management.
- chowdsp_plugin_utils
  - Base class for audio effect plugin.
  - Base class for synth plugin.
  - Utility class for managing plugin presets.
  - Utility functions for creating parameter layouts.
  - Utility class for "forwarding" parameters from one processor to another.

For complete documentation, see the [API docs](https://ccrma.stanford.edu/~jatin/chowdsp/chowdsp_utils).

## Usage

If you are using JUCE with CMake, simply add this repository as a subdirectory after adding JUCE to your CMake project.

```cmake
add_subdirectory(JUCE)
add_subdirectory(chowdsp_utils)

...
target_link_libraries(MyTarget PUBLIC
    juce::juce_audio_utils
    juce::juce_dsp
    # other JUCE libraries...
    chowdsp_dsp
    chowdsp_gui
    chowdsp_plugin_utils
)
```

Alternatively, you may add these modules from the repository directory from the Projucer.

## License

`chowdsp_utils` is open source, and is licensed under the GPLv3.
Enjoy!
