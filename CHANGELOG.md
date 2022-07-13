# Changelog

All notable changes to this project will be documented in this file.

## [UNRELEASED]
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
  - Inproved type-checking by using `type_info` instead of string

## [1.0.0] 2022-05-19
- Initial release.
