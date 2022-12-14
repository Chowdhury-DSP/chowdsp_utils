#include <juce_core/juce_core.h>
#include <chowdsp_core/chowdsp_core.h>

struct Test
{
    inline static int x = 4;
    const std::string_view y = "asdklfjhasljk";
    float zz = 0.0f;
};

CHOWDSP_CHECK_HAS_MEMBER (HasX, x)
CHOWDSP_CHECK_HAS_MEMBER (HasY, y)
CHOWDSP_CHECK_HAS_MEMBER (HasZ, z)
CHOWDSP_CHECK_HAS_MEMBER (HasZZ, zz)

int main()
{
    static_assert (! HasX<Test>, "x is static!");
    static_assert (HasY<Test>, "y should exist!");
    static_assert (! HasZ<Test>, "z should not exist!");
    static_assert (HasZZ<Test>, "zz should exist!");

    Test t {};

    return 0;
}
