#include <juce_core/juce_core.h>
#include <chowdsp_core/chowdsp_core.h>

struct Test
{
    inline static int x = 4;
    static constexpr std::string_view y = "asdklfjhasljk";
};

CHOWDSP_CHECK_HAS_STATIC_MEMBER (HasX, x)
CHOWDSP_CHECK_HAS_STATIC_MEMBER (HasY, y)
CHOWDSP_CHECK_HAS_STATIC_MEMBER (HasZ, z)

int main()
{
    static_assert (HasX<Test>, "x should exist!");
    static_assert (HasY<Test>, "y should exist!");
    static_assert (! HasZ<Test>, "z should not exist!");

    Test t {};

    return 0;
}
