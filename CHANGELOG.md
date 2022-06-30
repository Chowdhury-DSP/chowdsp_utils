# Changelog

All notable changes to this project will be documented in this file.

## [UNRELEASED]
- Added extensions for building CLAP plugins with CLAP-specific behaviour.
- Added improved State Variable Filter with more filter types.
- Added `chowdsp_waveshapers` module, including anti-aliased waveshapers.
- Replaced Bilinear Tranform code with more general conformal maps.
- Added utilities for serializing/deserializing data.
- Added `PluginDiagnosticInfo::getDiagnosticsString()`.
- LNFAllocator:
  - Don't replace existing look and feels
  - Hash types by type_info instead of string

## [1.0.0] 2022-05-19
- Initial release.
