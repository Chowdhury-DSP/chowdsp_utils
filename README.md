# chowdsp_utils

[![Test](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/run-tests.yml/badge.svg)](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/run-tests.yml)
[![Examples](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/examples.yml/badge.svg)](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/examples.yml)
[![Benchmarks](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/bench.yml/badge.svg)](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/bench.yml)
[![Code-Quality](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/code-quality.yml/badge.svg)](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/code-quality.yml)
[![Docs](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/docs.yml/badge.svg)](https://ccrma.stanford.edu/~jatin/chowdsp/chowdsp_utils)
[![codecov](https://codecov.io/gh/Chowdhury-DSP/chowdsp_utils/branch/master/graph/badge.svg?token=84B35MB5QS)](https://codecov.io/gh/Chowdhury-DSP/chowdsp_utils)

This repository contains JUCE modules with utilities for building Chowdhury DSP plugins.

There are a handful of simple examples in the [`examples/`](https://github.com/Chowdhury-DSP/chowdsp_utils/tree/master/examples) directory.

For complete documentation, see the [API docs](https://ccrma.stanford.edu/~jatin/chowdsp/chowdsp_utils).

## Usage

If you are using JUCE with CMake, simply add this repository as a subdirectory after adding JUCE to your CMake project.

```cmake
add_subdirectory(JUCE)
add_subdirectory(chowdsp_utils)

target_link_libraries(MyTarget PUBLIC
    juce::juce_audio_utils
    juce::juce_dsp
    # other JUCE modules...
    chowdsp::chowdsp_dsp
    chowdsp::chowdsp_gui
    chowdsp::chowdsp_plugin_base
    # Other modules and libraries...
)
```

Alternatively, you may add these modules from the repository directory using the Projucer.

## Examples

 If you would like to build the example plugins included in this repository, you may clone
the repository, and use the following CMake commands:

```bash
cmake -Bbuild -DCHOWDSP_ENABLE_EXAMPLES=ON
cmake --build build --target $EXAMPLE_TARGET
```

where `$EXAMPLE_TARGET` is the name of the target you'd like to build,
for example `SimpleEQ_Standalone`.

## Modules

### Dependencies

The modules in this repository are mostly dependent on C++17 or later, as well as JUCE version 6 or later.

There are a few other dependencies as well, some of which are bundled internally within
the repository, and others which must be managed externally. In either case, you must be
sure to abide by the license of each module, as well as whichever libraries are being used.

### Common Modules

`chowdsp_core` (BSD)
- `DoubleBuffer`: A circular buffer which always maintans a contiguous block of data.
- `TupleHelpers`: Useful methods for working with tuple-like data structures.
- `AtomicHelpers`: Useful methods for working with atomics.

`chowdsp_json` (BSD)
- A thin wrapper around [`nlohmann::json`](https://github.com/nlohmann/json) (MIT license, included internally).

`chowdsp_reflection` (BSD)
- A thin wrapper around [`boost::pfr`](https://github.com/boostorg/pfr) (Boost license, included internally).

`chowdsp_serialization` (BSD)
- Tools for serializing/deserializing data, using JSON or XML formats.

### DSP Modules

`chowdsp_dsp_data_structures` (GPLv3)
- `COLAProcessor`: A base class for doing Constant Overlap-Add processing.
- `LookupTableTransform`: Some modifications on `juce::dsp::LookupTableTransform`.
- `RebufferedProcessor`: A processor which rebuffers the input to have a constant block size.
- `SmoothedBufferValue`: A buffered version of `juce::SmoothedValue`.

`chowdsp_dsp_utils` (GPLv3)
- Utilities for working with convolution and impulse responses.
- `DelayLine`: A re-implementation of `juce::dsp::DelayLine` with more interpolation options.
- `PitchShifter`: Simple pitch-shifting effect using a ring buffer with two read pointers.
- Useful classes for modal signal processing.
- Various classes for integer or non-integer resampling
  - Optionally depends on [libsamplerate](https://github.com/libsndfile/libsamplerate). User must link with libsamplerate externally, and define `CHOWDSP_USE_LIBSAMPLERATE=1`
- Sources: "magic circle" sine wave oscillator, anti-aliased saw and square wave oscillators, noise generator.
- A few other useful processors.

`chowdsp_eq` (GPLv3)
- `EQBand`: A single EQ band.
- `EQProcessor`: A collection of EQ bands.
- `LinearPhaseEQ`: Constructs a linear phase EQ from a give prototype EQ.

`chowdsp_filters` (GPLv3)
- Basic first and second order filters (HPF/LPF/BPF, shelving filters, peaking filters, notch filters).
- Some higher-order filters (Butterworth, Chebyshev (Type II), Elliptic).
- `StateVariableFilter`: A modified version of `juce::dsp::StateVariableTPTFilter`.
- `ModFilterWrapper`: Turns any biquad filter into a State Variable Filter.
- `FIRFilter`: An FIR filter with SIMD optimizations. 

`chowdsp_math` (BSD)
- `FloatVectorOperations`: Some extensions on `juce::FloatVectorOperations`.
- `Polynomials`: Methods for evaluating polynomials with Horner's method or Estrin's scheme.
- A few other useful math operations.

`chowdsp_reverb` (GPLv3)
- Some modular template classes for constructing Feedback Delay Network reverbs.

`chowdsp_simd` (BSD)
- A wrapper around [XSIMD](https://github.com/xtensor-stack/xsimd) (BSD, included internally).
- `SIMDSmoothedValue`: A SIMD specialization of `juce::SmoothedValue`.
- A few other extea SIMD math functions.

## GUI Modules

`chowdsp_gui` (GPLv3)
- Custom `LookAndFeel`.
- Helper class for optionally using OpenGL.
- Helper classes for working with long-presses and popup menus on touch screens.
- Custom tooltip viewer.
- Component to display plugin info (version, format, etc.).
- Component for controlling preset management.

`chowdsp_foleys` (GPLv3)
- Interface between `chowdsp_gui` and [`foleys_gui_magic`](https://github.com/ffAudio/foleys_gui_magic) (must be linked externally).

## Music Modules

`chowdsp_rhythm` (BSD)
- Utilities for working with rhythms, and converting between a rhythm and a unit of time.

## Plugin Utility Modules

`chowdsp_clap_extensions` (BSD)
- `CLAPExtensions::CLAPProcessorExtensions`: implements `clap_direct_process` with parameter modulation.
- `CLAPExtensions::CLAPInfoExtensions`: implements `getPluginTypeString()`.
- `ModParameterMixin`: interface for supporting CLAP parameter modulation.

`chowdsp_parameters` (BSD)
- `ParamUtils`: Useful methods for creating parameters.
- `ForwardingParameter`: A parameter that forwards on a parameter from another processor.

`chowdsp_plugin_base` (GPLv3)
- Bass classes for creating audio effect or synthesizer plugins.

`chowdsp_plugin_utils` (GPLv3)
- `FileListener`: A listener which triggers a callback whenever a file is changed.
- `PluginLogger`: A logging system which can be used within a plugin.
- `SharedPluginSettings`: A shared object for managing settings which apply ot all instances of a plugin.
- `SharedLNFAllocator`: A shared object for managing `juce::LookAndFeel` classes.
- `AudioUIBackgroundThread`: A thread class which accepts data from the audio thread, and performs a background task (often useful for creating meters).

`chowdsp_presets` (BSD)
- A system for managing plugin presets.

`chowdsp_version` (BSD)
- Utilities for managing the version of an app or plugin.

## Development

The development environment for this repository expects the following
directory structure:
```
|- JUCE
|- modules
  |- foleys_gui_magic
  |- chowdsp_utils
```

### Building Module Tests

To build the module tests, run:
```bash
cmake -Bbuild -DCHOWDSP_ENABLE_TESTING=ON
cmake --build build --target $TEST_TARGET
```

where `$TEST_TARGET` is the name of the test target you'd like to build.

If you would like to build the tests with flags for analyzing code coverage,
add `-DCODE_COVERAGE=ON` to the CMake configure step.

### Building Module Benchmarks

Toe build the module benchmarks, run:
```bash
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release -DCHOWDSP_ENABLE_BENCHMARKS=ON
cmake --build build --config Release $BENCH_TARGET
```

where `$BENCH_TARGET` is the name of the benchmark you would like to build.

## License

Each module in this repository has its own unique license. If you would like
to use code from one of the modules, please check the license of that particular
module.

If you are making a proprietary or closed source app and would like to use
code from a module that is under a GPL-style license, please contact
chowdsp@gmail.com for non-GPL licensing options.

All non-module code in this repository (tests, examples, benchmarks, etc.)
is licensed under the GPLv3.
