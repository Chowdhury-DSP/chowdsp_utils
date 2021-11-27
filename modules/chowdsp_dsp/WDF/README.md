# Wave Digital Filters

This directory contains a standalone Wave Digital Filter (WDF) library. Examples demonstrating how to use the library
can be found in a [separate repository](https://github.com/jatinchowdhury18/WaveDigitalFilters).

Since the WDF library is header-only, the only requirement to using the library is to include the correct header file
for your use case:
- If you would like to build a WDF defined at run-time, you should `#include "wdf.h"`.
- If you would like to build a WDF defined at compile-time, you should `#include "wdf_t.h"`.
- If you would like to build a WDF using R-type adaptors (still experimental), you should `#include "r_type.h"`.
