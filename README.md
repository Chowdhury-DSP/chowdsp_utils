# chowdsp_utils

![CI](https://github.com/Chowdhury-DSP/chowdsp_utils/workflows/CI/badge.svg)
[![License](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://opensource.org/licenses/GPL-3.0)
[![codecov](https://codecov.io/gh/Chowdhury-DSP/chowdsp_utils/branch/master/graph/badge.svg?token=84B35MB5QS)](https://codecov.io/gh/Chowdhury-DSP/chowdsp_utils)

This repository contains JUCE modules with utilities for building Chowdhury DSP plugins.

There are currently 3 modules each containing the following utilities:
- chowdsp_dsp
  - Convolution: Helpful classes and methods for working with IRs and real-time convolution
  - Delay Line: similar to `juce::dsp::DelayLine` but with 5th-order Lagrange interpolation and Sinc interpolation.
  - Pitch Shifter: using a ring buffer with two read pointers.
  - Filters: functions for bilinear transform, IIR filters with static order, and an optimized State Variable Filter.
  - Modal: modal filter/oscillator using Max Mathews Phasor Filter, and a parallel filterbank to use those modal filters.
  - Sources: "magic circle" sine wave oscillator, anti-aliased saw and square oscillators, noise generator.
  - Resampling: classes for doing integer or non-integer sample rate conversion, and processing audio at a target sample rate.
  - Wave Digital Filters: linear circuit elements, series/parallel junctions, diodes, R-Type adaptors, and a few others.
  - SIMD Utils: Some extensions on `juce::dsp::SIMDRegister`.
  - A few other odds and ends.
- chowdsp_gui
  - Custom `LookAndFeel`.
  - Helper class for optionally using OpenGL.
  - Helper classes for working with long-presses and popup menus on touch screens.
  - Custom tooltip viewer.
  - Component to display plugin info (version, format, etc.).
  - Component for controlling preset management.
  - Interfaces with [Plugin GUI Magic](https://github.com/Chowdhury-DSP/foleys_gui_magic/tree/chowdsp) (a slightly older version).
- chowdsp_plugin_utils
  - Base class for audio effect plugin.
  - Base class for synth plugin.
  - Utility class for managing plugin presets.
  - Utility functions for creating parameter layouts.
  - Utility class for "forwarding" parameters from one processor to another.
  - Shared resource class for managing `LookAndFeel` allocation.
  - Shared resource class for managing global plugin settings.
  - JUCE-based Wrapper around `nlohmann::json`.
  - Thread class for running a task that reads from the audio thread, and reports info to the UI thread.

There's also a handful of simple examples in the [`examples/`](https://github.com/Chowdhury-DSP/chowdsp_utils/tree/master/examples) directory.

For complete documentation, see the [API docs](https://ccrma.stanford.edu/~jatin/chowdsp/chowdsp_utils).

## Usage

If you are using JUCE with CMake, simply add this repository as a subdirectory after adding JUCE to your CMake project.

```cmake
add_subdirectory(JUCE)
add_subdirectory(chowdsp_utils)

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
