# chowdsp_utils

[![Version](https://img.shields.io/github/v/release/Chowdhury-DSP/chowdsp_utils?color=gold&include_prereleases&label=Release&logo=Github)](https://github.com/Chowdhury-DSP/chowdsp_utils/releases/latest)
[![Test](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/run-tests.yml/badge.svg)](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/run-tests.yml)
[![Examples](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/examples.yml/badge.svg)](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/examples.yml)
[![Benchmarks](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/bench.yml/badge.svg)](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/bench.yml)
[![Code-Quality](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/code-quality.yml/badge.svg)](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/code-quality.yml)
[![Docs](https://github.com/Chowdhury-DSP/chowdsp_utils/actions/workflows/docs.yml/badge.svg)](https://ccrma.stanford.edu/~jatin/chowdsp/chowdsp_utils)
[![codecov](https://codecov.io/gh/Chowdhury-DSP/chowdsp_utils/branch/master/graph/badge.svg?token=84B35MB5QS)](https://codecov.io/gh/Chowdhury-DSP/chowdsp_utils)
[![sonarscan](https://sonarcloud.io/api/project_badges/measure?project=Chowdhury-DSP_chowdsp_utils&metric=alert_status)](https://sonarcloud.io/dashboard?id=Chowdhury-DSP_chowdsp_utils)

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
    chowdsp::chowdsp_dsp_utils
    chowdsp::chowdsp_gui
    chowdsp::chowdsp_plugin_base
    # Other modules and libraries...
)
```

Alternatively, you may add these modules from the repository directory using the Projucer.

If you are using a non-JUCE CMake project, it is possible to create your own static library
from the DSP modules in this repository:
```cmake
add_subdirectory(chowdsp_utils)

# create static library based on these modules
setup_chowdsp_lib(chowdsp_lib               # Name of the static library
    MODULES chowdsp_math chowdsp_dsp_utils  # DSP modules that should be included in the library...
)

# link the static library to your project
target_link_libraries(MyCoolProject PRIVATE chowdsp_lib)
```

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

The modules in this repository are mostly dependent on C++17 or later. Modules not
in the "Common" or "DSP" categories also depend on JUCE 6 or later.

There are a few other dependencies as well, some of which are bundled internally within
the repository, and others which must be managed externally. In either case, you must be
sure to abide by the license of each module, as well as whichever libraries are being used.

### Common Modules

`chowdsp_core` (BSD)
- `AtomicHelpers`: Useful methods for working with atomics.
- `MemoryUtils`: Helpful methods to check if blocks of memory alias with each other.
- `TypeTraits`: Type traits for checking if a class has a given method, if a type is a container, and so on.
- `ScopedValue`: A stack value that writes its value back to a more permanent variable when it goes out of scope.
- Includes the following internal dependencies:
  - [`types_list`](https://github.com/jatinchowdhury18/types_list) (MIT license).
  - [`span-lite`](https://github.com/martinmoene/span-lite) (BSL-1.0 license).

`chowdsp_data_structures` (BSD)
- `DoubleBuffer`: A circular buffer which always maintains a contiguous block of data.
- `TupleHelpers`: Useful methods for working with tuple-like data structures.
- `OptionalPointer`: A pointer which may or may not own the data it points to.
- `LocalPointer`: An owning pointer which constructs the object it points to using its own local memory.
- `SmallVector`: A mostly STL-compatible vector implementation, with a small-string-style optimization.
- `ArenaAllocator`: A simple arena allocator.
- `ChainedArenaAllocator`: A "growable" arena allocator.
- `PoolAllocator`: A simple pool allocator.
- Includes the following internal dependencies:
  - [`short_alloc`](https://howardhinnant.github.io/stack_alloc.html) (MIT license).

`chowdsp_json` (BSD)
- A thin wrapper around [`nlohmann::json`](https://github.com/nlohmann/json) (MIT license, included internally).

`chowdsp_listeners` (BSD)
- A thin wrapper around [`rocket`](https://github.com/tripleslash/rocket) (Public Domain, included internally).

`chowdsp_logging` (BSD)
- A thin wrapper around [`spdlog`](https://github.com/gabime/spdlog) (MIT License, included internally).

`chowdsp_reflection` (BSD)
- Includes the following internal dependencies:
  - [`boost::pfr`](https://github.com/boostorg/pfr) (Boost license).
  - [`nameof`](https://github.com/Neargye/nameof) (MIT license).
  - [`magic_enum`](https://github.com/Neargye/magic_enum) (MIT license).

`chowdsp_serialization` (BSD)
- Tools for serializing/deserializing data, using JSON or XML formats.

### DSP Modules

`chowdsp_buffers` (BSD)
- `Buffer`: A basic audio buffer, which supports SIMD data types.
- `StaticBuffer`: A basic audio buffer using local (rather than heap-allocated) memory.
- `BufferView`: A non-owning "view" over the data in an audio buffer (compatible with `juce::AudioBuffer`).
- `buffer_iters`: Handy C++ iterators for iterating over buffers in different ways.

`chowdsp_compressor` (GPLv3)
- `LevelDetector`: A multi-modal level detector.
- `GainComputer`: A multi-modal compressor gain computer.
- `MonoCompressor`: A simple compressor with monophonic gain reduction.

`chowdsp_dsp_data_structures` (GPLv3)
- `LookupTableTransform`: Some modifications on `juce::dsp::LookupTableTransform`.
- `LookupTableCache`: A cache for storing lookup tables, so they don't need to be re-computed.
- `SmoothedBufferValue`: A buffered version of `juce::SmoothedValue`.
- `COLAProcessor`: A base class for doing Constant Overlap-Add processing.
- `RebufferedProcessor`: A processor which rebuffers the input to have a constant block size.
- Wrappers around `moodycamel`'s [lock-free queues](https://github.com/cameron314/readerwriterqueue) (modified BSD)

`chowdsp_dsp_utils` (GPLv3)
- Utilities for working with convolution and impulse responses.
- `DelayLine`: A re-implementation of `juce::dsp::DelayLine` with more interpolation options.
- `PitchShifter`: Simple pitch-shifting effect using a ring buffer with two read pointers.
- Various classes for integer or non-integer resampling
  - Optionally depends on [libsamplerate](https://github.com/libsndfile/libsamplerate). User must link with libsamplerate externally, and define `CHOWDSP_USE_LIBSAMPLERATE=1`
- A few other useful processors.

`chowdsp_eq` (GPLv3)
- `EQBand`: A single EQ band.
- `EQProcessor`: A collection of EQ bands.
- `LinearPhaseEQ`: Constructs a linear phase EQ from a given prototype EQ.

`chowdsp_filters` (GPLv3)
- Basic first and second order filters (HPF/LPF/BPF, shelving filters, peaking filters, notch filters).
- Higher-order filters (Butterworth, Chebyshev (Type II), Elliptic).
- `StateVariableFilter`: A modified version of `juce::dsp::StateVariableTPTFilter` with more filter types and better performance.
- `ModFilterWrapper`: Turns any biquad filter into a State Variable Filter.
- `FIRFilter`: An FIR filter with SIMD optimizations.
- `WernerFilter`: Emulation of a generalized Octave-CAT-style SVF, as proposed by Kurt Werner.
- `ARPFilter`: Emulation of the ARP 1047 multi-mode filter.
- `FractionalOrderFilter`: A filter with a slope between 0-3 dB / octave.
- `ConformalMaps`: Conformal maps to use for designing filters, including the bilinear and alpha transforms.
- `FilterChain`: Utility template class for chaining multiple similar filters in series.

`chowdsp_math` (BSD)
- Internally includes [gcem](https://github.com/kthohr/gcem) (Apache 2.0)
- `FloatVectorOperations`: Some extensions on `juce::FloatVectorOperations`.
- `BufferMath`: Math operations which can be applied to audio buffers.
- `Polynomials`: Methods for evaluating polynomials with Horner's method or Estrin's scheme.
- `Power`: Fast integer exponential operations.
- A few other useful math operations.

`chowdsp_modal_dsp` (GPLv3)
- `ModalFilter`: Implementation of the Max Mathews "phasor filter".
- `ModalFilterBank`: Vectorized modal filterbank, which can be used to implement large-scale modal models.

`chowdsp_reverb` (GPLv3)
- Some modular template classes for constructing Feedback Delay Network of Dattorro reverbs.

`chowdsp_simd` (BSD)
- A wrapper around [XSIMD](https://github.com/xtensor-stack/xsimd) (BSD, included internally).
- `SIMDSmoothedValue`: A SIMD specialization of `juce::SmoothedValue`.
- A few other extra SIMD math functions.

`chowdsp_sources` (GPLv3)
- `SineWave`: a "magic circle" sine wave oscillator.
- Other basic oscillators implemented with aliasing suppression:
  - `SawtoothWave`
  - `SquareWave`
  - `TriangleWave`
- `Noise`: a noise generator with options for Guassian (normal), uniform, and pink noise.

`chowdsp_waveshapers` (GPLv3)
- Basic waveshaping processes implemented with integrated waveshaping (ADAA), including:
  - `ADAAHardClipper`
  - `ADAATanhClipper`
  - `ADAASoftClipper`: A polynomial soft clipper
  - `ADAAFullWaveRectifier`
- `WestCoastFolder`: An emulation of the wavefolding circuit found from the Buchla 259
- `WaveMultiplier`: An emulation of Serge Tcherepin's Wave Multiplier circuit

## GUI Modules

`chowdsp_gui` (GPLv3)
- Custom `LookAndFeel`.
- Helper class for optionally using OpenGL.
- Helper classes for working with long-presses and popup menus on touch screens.
- Helper class for connecting UI elements to host-specific parameter information.
- Custom tooltip viewer.
- Component to display plugin info (version, format, etc.).
- Component for controlling preset management.

`chowdsp_visualizers` (GPLv3)
- `EqualizerPlot`: UI component which can be extended to plot filter frequency responses.

`chowdsp_foleys` (GPLv3)
- Interface between `chowdsp_gui` and [`foleys_gui_magic`](https://github.com/ffAudio/foleys_gui_magic) (must be linked externally).

## Music Modules

`chowdsp_rhythm` (BSD)
- Utilities for working with rhythms, and converting between a rhythm and a unit of time.

## Plugin Utility Modules

`chowdsp_clap_extensions` (BSD)
- Helper classes for implementing features specific to the CLAP plugin format.
- `CLAPExtensions::CLAPInfoExtensions`: implements `getPluginTypeString()`.
- `ModParameterMixin`: interface for supporting CLAP parameter modulation.

`chowdsp_parameters` (BSD)
- `ParamUtils`: Useful methods for creating parameters.
- `ForwardingParameter`: A parameter that forwards on a parameter from another processor.

`chowdsp_plugin_state` (BSD)
- `PluginState`: Replacement for `juce::AudioProcessorValueTreeState`.
- Parameter "attachments" for connecting the plugin state to various UI elements.

`chowdsp_plugin_base` (GPLv3)
- Base classes for creating audio effect or synthesizer plugins.

`chowdsp_plugin_utils` (GPLv3)
- `FileListener`: A listener which triggers a callback whenever a file is changed.
- `SharedPluginSettings`: A shared object for managing settings which apply to all instances of a plugin.
- `SharedLNFAllocator`: A shared object for managing `juce::LookAndFeel` classes.
- `AudioUIBackgroundThread`: A thread class which accepts data from the audio thread, and performs a background task (often useful for creating meters).
- `DeferredAction`: A helper class for queueing an action to be run on the main thread (real-time safe).

`chowdsp_presets` (BSD)
- A system for managing plugin presets.

`chowdsp_preset_v2` (BSD)
- An updated preset management system, compatible with `chowdsp_plugin_state`.

`chowdsp_version` (BSD)
- Utilities for managing the version of an app or plugin.

`chowdsp_fuzzy_search` (BSD)
- A tag-based fuzzy searching system, based on [fuzzysearchdatabase](https://bitbucket.org/j_norberg/fuzzysearchdatabase).

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
