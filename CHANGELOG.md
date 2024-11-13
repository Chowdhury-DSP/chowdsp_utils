# Changelog

All notable changes to this project will be documented in this file.

## [2.3.0] 2024-11-13
- Added `chowdsp_fuzzy_search` module.
- Added data structures: `chowdsp::ComponentArena`, `chowdsp::EnumMap`, `chowdsp::OptionalRef` and `chowdsp::OptionalArray`.
- Added data structures: `chowdsp::PoolAllocator` and `chowdsp::ObjectPool`.
- Added `chowdsp::SemitonesParameter`.
- Added `chowdsp::BufferMultiple`.
- Added parameters assignment methods.
- Improved backend for `chowdsp_plugin_state`.
- Improved `chowdsp::ArenaAllocator` and `chowdsp::ChainedArenaAllocator`.
- Improved `chowdsp::AbstractTree`.
- Improved `chowdsp::ParametersView`.
- Improved `chowdsp::logging`.
- Various memory-related improvements.
- Updated PFR internal dependency.
- Updated usage of `juce::Font` for compatibility with JUCE 8.
- `chowdsp_compressor`: Allow level detector out-of-place processing.
- `chowdsp::RebufferedProcessor` now supports work splitting.
- Fixed moodycamel header clashes.

## [2.2.0] 2024-02-19
- Refactored `chowdsp_buffers` module out of `chowdsp_dsp_data_structures`.
- Refactored `chowdsp_data_structures` module out of `chowdsp_core`, and added new data structures.
- Added `chowdsp_compressor` module.
- Added `chowdsp_logging` module.
- Added `chowdsp::zip_multi`.
- Added `chowdsp::buffer_iters::samples` and `chowdsp::buffer_iters::zip_channels`.
- Added `chowdsp::ArenaAllocator` and `chowdsp::ChainedArenaAllocator`.
- Added `chowdsp::UIToAudioPipeline`.
- Added `chowdsp::CrossoverFilter`.
- Added `chowdsp::NoiseSynth`.
- Added `chowdsp::OvershootLimiter`.
- Added `chowdsp::make_array`.
- Added `chowdsp::EndOfScopeAction` and `chowdsp::runAtEndOfScope`.
- Added `chowdsp::RandomFloat`.
- Added `chowdsp::FIRPolyphaseInterpolator` and `chowdsp::FIRPolyphaseDecimator`.
- Added `chowdsp::SmallMap`.
- Added `chowdsp::WidthPanner`.
- `chowdsp_visualizers`: Added `WaveshaperPlot`.
- `chowdsp_buffers`: Improved constructor and type compatibility.
- `chowdsp_dsp_utils`: Optimized `chowdsp::Upsampler` and `chowdsp::Downsampler`.
- `chowdsp_serialization`: Added specializations for serializing enums and json.
- `chowdsp_clap_extensions`: Added helpers for preset discovery.
- `chowds::DelayLine`: Added `free()` and `processBlock()` methods.
- `chowdsp::SynthBase`: No longer clears the audio buffer before passing it into `processSnyth()`.
- `chowdsp_presets_v2`: Refactored preset save/load logic into `chowdsp::PresetSaverLoader`.
- `chowdsp::CoefficientCalculationMode::Decramped`: Improved stability for decramped filters with low Q.
- `chowdsp::StringLiteral`: Improved construction.
- `chowdsp::SineWave`: Added `processSampleQuadrature()`.
- `chowdsp::Polynomials::estrin`: Fixed address sanitizer error.
- Improved memory alignment handling for structures that deal with low-level memory.
- Improved compatibility for targets without SIMD support.

## [2.1.0] 2023-03-04
- Added `chowdsp_visualizers` module.
- Added `chowdsp_presets_v2` module.
- Added `chowdsp::StringLiteral`.
- Added `chowdsp::buffer_iters`.
- Added `chowdsp::AdditiveOscillator`.
- Added `chowdsp::ThreeWayCrossoverFilter`.
- `chowdsp::Gain`: Fixed reset() to reset the smoother to the target gain value.
- BypassProcessor: Added template specialization with no latency compensation.
- `chowdsp::BufferMath`: Added multiplyBufferData() function.
- `chowdsp::BufferView`: Added constructor for 1d-array.
- `chowdsp::LinearPhaseEQ`: Prototype EQs must now define their floating point type.
- Moved `Version` to `chowdsp` namespace, and made constexpr-able.

## [2.0.0] 2022-12-22
- Added `chowdsp_plugin_state` for managing plugin state.
- Added Dattorro reverb classes.
- Added `enumerate` and `zip` iterators.
- Added CLAP note name support to `chowdsp::SynthBase`.
- Added `TupleHelpers::visit_at`.
- Added `chowdsp::OptionalPointer`.
- Added `chowdsp::toString()` for converting `std::string_view` to `juce::String`.
- Added `chowdsp::HostContextProvider` for connecting UI elements to host parameter context info.
- Added `chowdsp::TweaksFile` for prototyping and fine-tuning with live changes.
- Added `chowdsp::Rectangle`: a `constexpr`-capable clone of `juce::Rectangle`.
- Improved const-correctness of `chowdsp::BufferView`.
- Improved flexibility for saving/loading JSON objects.

## [1.3.0] 2022-11-03
- Added `LinkwitzRileyFilter` class.
- Added `TypesList` meta-programming helpers.
- Added `Ratio` class for creating template parameter floats.
- Added `BufferMath::sanitizeBuffer` method.
- Added `bind_front` and `bind_back` helper methods.
- Added `algebraicSigmoid` method.
- Added `StaticBuffer` class.
- Added `Reverb::ConvolutionDiffuser` class.
- Updated XSIMD to version 10.0.0rc.
- Updated elliptic filter template arguments to accept a `Ratio` for the passband ripple.

## [1.2.0] 2022-09-03
- Added filters: Kurt Werner's generalized SVF, ARP 1047 filter.
- Added waveshapers: Buchla wavefolder, Serge Wave Multiplier, full wave rectifier.
- Added triangle wave oscillator.
- Added support for building DSP modules using the Teensy toolchain.
- Added `DeferredAction` for running any action on the message thread without locking.
- Added wrapper for `moodycamel::ReaderWriterQueue` and `moodycamel::ConcurrentQueue`.
- Added wrapper for `rocket` library for listeners/broadcasters.
- Added compile-time maths functions with GCEM.
- Added methods for setting/clearing groups of parameters in `ForwardingParametersManager`.
- Fixed out-of-order preset swapping in `PresetManager`.
- Fixed `StateVariableFilter` vector initialization bug.
- Updated `ButterworthFilter` to support odd filter orders.
- Updated `CoefficientCalculators` to support decramped biquad filters using Vicanek's method.
- Updated `nlohmann::json` to version 3.11.1.
- Updated `VariableOversampling` to be compatible with JUCE 7 parameter version hints.

## [1.1.0] 2022-07-14
- Removed dependency on JUCE for most DSP modules.
- Added extensions for building CLAP plugins with CLAP-specific behaviour.
- Added improved State Variable Filter with more filter types.
- Added `chowdsp_waveshapers` module, including anti-aliased waveshapers.
- Replaced Bilinear Tranform code with more general conformal maps.
- Added utilities for serializing/deserializing data.
- Added `PluginDiagnosticInfo::getDiagnosticsString()`.
- Math: Added a dilogarithm implementation.
- `chowdsp::Upsampler` and `chowdsp::Downsampler` use template arguments for filter type instead of filter order.
- LNFAllocator:
  - Fixed bug where existing LookAndFeel classes would be replaced
  - Improved type-checking by using `type_info` instead of string

## [1.0.0] 2022-05-19
- Initial release.
