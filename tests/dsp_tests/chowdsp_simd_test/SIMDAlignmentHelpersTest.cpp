#include <CatchUtils.h>
#include <chowdsp_simd/chowdsp_simd.h>

using namespace chowdsp::SIMDUtils;

TEMPLATE_TEST_CASE ("SIMD Alignment Helpers Test", "[dsp][simd]", float, double)
{
    SECTION ("isAligned() Test")
    {
        alignas (defaultSIMDAlignment) float data[4];
        REQUIRE (isAligned (data));
        REQUIRE (! isAligned (data + 1));
    }

    SECTION ("getNextAlignedPtr() Test")
    {
        alignas (defaultSIMDAlignment) float data[32];

        const auto unalignedPtr = data + 1;
        REQUIRE (! isAligned (unalignedPtr));

        const auto nextAlignedPtr = getNextAlignedPtr (unalignedPtr);
        REQUIRE (isAligned (nextAlignedPtr));
        REQUIRE (nextAlignedPtr - unalignedPtr > 0);
    }
}
