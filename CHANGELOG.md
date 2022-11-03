# Changelog

All notable changes to this project will be documented in this file.

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
