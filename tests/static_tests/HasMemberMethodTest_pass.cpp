#include <juce_core/juce_core.h>
#include <chowdsp_core/chowdsp_core.h>

struct Test
{
    void memberMethod() {}
    void oneArgMemberMethod (int) {}
    void twoArgMemberMethod (int, float) {}
    int nonVoidMemberMethod() { return 0; }
    int oneArgNonVoidMemberMethod (int) { return 0; }
    int twoArgNonVoidMemberMethod (int, float) { return 0; }
};

CHOWDSP_CHECK_HAS_METHOD (HasMemberMethod, memberMethod)
CHOWDSP_CHECK_HAS_METHOD (HasMemberMethod2, memberMethod2)
CHOWDSP_CHECK_HAS_METHOD (HasOneArgMemberMethod, oneArgMemberMethod, int {})
CHOWDSP_CHECK_HAS_METHOD (HasOneArgMemberMethod2, oneArgMemberMethod2, int {})
CHOWDSP_CHECK_HAS_METHOD (HasTwoArgMemberMethod, twoArgMemberMethod, int {}, float {})
CHOWDSP_CHECK_HAS_METHOD (HasTwoArgMemberMethod2, twoArgMemberMethod2, int {}, float {})
CHOWDSP_CHECK_HAS_METHOD (HasNonVoidMemberMethod, nonVoidMemberMethod)
CHOWDSP_CHECK_HAS_METHOD (HasNonVoidMemberMethod2, nonVoidMemberMethod2)
CHOWDSP_CHECK_HAS_METHOD (HasOneArgNonVoidMemberMethod, oneArgNonVoidMemberMethod, int {})
CHOWDSP_CHECK_HAS_METHOD (HasOneArgNonVoidMemberMethod2, oneArgNonVoidMemberMethod2, int {})
CHOWDSP_CHECK_HAS_METHOD (HasTwoArgNonVoidMemberMethod, twoArgNonVoidMemberMethod, int {}, float {})
CHOWDSP_CHECK_HAS_METHOD (HasTwoArgNonVoidMemberMethod2, twoArgNonVoidMemberMethod2, int {}, float {})

int main()
{
    static_assert (HasMemberMethod<Test>, "memberMethod should exist!");
    static_assert (! HasMemberMethod2<Test>, "memberMethod2 should not exist!");

    static_assert (HasOneArgMemberMethod<Test>, "should exist!");
    static_assert (! HasOneArgMemberMethod2<Test>, "should not exist!");

    static_assert (HasTwoArgMemberMethod<Test>, "should exist!");
    static_assert (! HasTwoArgMemberMethod2<Test>, "should not exist!");

    static_assert (HasNonVoidMemberMethod<Test>, "should exist!");
    static_assert (! HasNonVoidMemberMethod2<Test>, "should not exist!");

    static_assert (HasOneArgNonVoidMemberMethod<Test>, "should exist!");
    static_assert (! HasOneArgNonVoidMemberMethod2<Test>, "should not exist!");

    static_assert (HasTwoArgNonVoidMemberMethod<Test>, "should exist!");
    static_assert (! HasTwoArgNonVoidMemberMethod2<Test>, "should not exist!");

    Test t {};

    return 0;
}
