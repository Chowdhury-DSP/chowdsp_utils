#include <juce_core/juce_core.h>
#include <chowdsp_core/chowdsp_core.h>

struct Test
{
    static void staticMethod() {}
    static void oneArgStaticMethod (int) {}
    static void twoArgStaticMethod (int, float) {}
    static int nonVoidStaticMethod() { return 0; }
    static int oneArgNonVoidStaticMethod (int) { return 0; }
    static int twoArgNonVoidStaticMethod (int, float) { return 0; }

    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasStaticMethod, staticMethod)
    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasStaticMethod2, staticMethod2)
    static_assert (HasStaticMethod<Test>, "staticMethod should exist!");
    static_assert (! HasStaticMethod2<Test>, "staticMethod2 should not exist!");

    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasOneArgStaticMethod, oneArgStaticMethod)
    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasOneArgStaticMethod2, oneArgStaticMethod2)
    static_assert (HasOneArgStaticMethod<Test>, "should exist!");
    static_assert (! HasOneArgStaticMethod2<Test>, "should not exist!");

    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasTwoArgStaticMethod, twoArgStaticMethod)
    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasTwoArgStaticMethod2, twoArgStaticMethod2)
    static_assert (HasTwoArgStaticMethod<Test>, "should exist!");
    static_assert (! HasTwoArgStaticMethod2<Test>, "should not exist!");

    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasNonVoidStaticMethod, nonVoidStaticMethod)
    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasNonVoidStaticMethod2, nonVoidStaticMethod2)
    static_assert (HasNonVoidStaticMethod<Test>, "should exist!");
    static_assert (! HasNonVoidStaticMethod2<Test>, "should not exist!");

    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasOneArgNonVoidStaticMethod, oneArgNonVoidStaticMethod)
    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasOneArgNonVoidStaticMethod2, oneArgNonVoidStaticMethod2)
    static_assert (HasOneArgNonVoidStaticMethod<Test>, "should exist!");
    static_assert (! HasOneArgNonVoidStaticMethod2<Test>, "should not exist!");

    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasTwoArgNonVoidStaticMethod, twoArgNonVoidStaticMethod)
    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasTwoArgNonVoidStaticMethod2, twoArgNonVoidStaticMethod2)
    static_assert (HasTwoArgNonVoidStaticMethod<Test>, "should exist!");
    static_assert (! HasTwoArgNonVoidStaticMethod2<Test>, "should not exist!");
};

CHOWDSP_CHECK_HAS_STATIC_METHOD (HasStaticMethod, staticMethod)
CHOWDSP_CHECK_HAS_STATIC_METHOD (HasStaticMethod2, staticMethod2)
CHOWDSP_CHECK_HAS_STATIC_METHOD (HasOneArgStaticMethod, oneArgStaticMethod)
CHOWDSP_CHECK_HAS_STATIC_METHOD (HasOneArgStaticMethod2, oneArgStaticMethod2)
CHOWDSP_CHECK_HAS_STATIC_METHOD (HasTwoArgStaticMethod, twoArgStaticMethod)
CHOWDSP_CHECK_HAS_STATIC_METHOD (HasTwoArgStaticMethod2, twoArgStaticMethod2)
CHOWDSP_CHECK_HAS_STATIC_METHOD (HasNonVoidStaticMethod, nonVoidStaticMethod)
CHOWDSP_CHECK_HAS_STATIC_METHOD (HasNonVoidStaticMethod2, nonVoidStaticMethod2)
CHOWDSP_CHECK_HAS_STATIC_METHOD (HasOneArgNonVoidStaticMethod, oneArgNonVoidStaticMethod)
CHOWDSP_CHECK_HAS_STATIC_METHOD (HasOneArgNonVoidStaticMethod2, oneArgNonVoidStaticMethod2)
CHOWDSP_CHECK_HAS_STATIC_METHOD (HasTwoArgNonVoidStaticMethod, twoArgNonVoidStaticMethod)
CHOWDSP_CHECK_HAS_STATIC_METHOD (HasTwoArgNonVoidStaticMethod2, twoArgNonVoidStaticMethod2)

int main()
{
    static_assert (HasStaticMethod<Test>, "staticMethod should exist!");
    static_assert (! HasStaticMethod2<Test>, "staticMethod2 should not exist!");

    static_assert (HasOneArgStaticMethod<Test>, "should exist!");
    static_assert (! HasOneArgStaticMethod2<Test>, "should not exist!");

    static_assert (HasTwoArgStaticMethod<Test>, "should exist!");
    static_assert (! HasTwoArgStaticMethod2<Test>, "should not exist!");

    static_assert (HasNonVoidStaticMethod<Test>, "should exist!");
    static_assert (! HasNonVoidStaticMethod2<Test>, "should not exist!");

    static_assert (HasOneArgNonVoidStaticMethod<Test>, "should exist!");
    static_assert (! HasOneArgNonVoidStaticMethod2<Test>, "should not exist!");

    static_assert (HasTwoArgNonVoidStaticMethod<Test>, "should exist!");
    static_assert (! HasTwoArgNonVoidStaticMethod2<Test>, "should not exist!");

    Test t {};

    return 0;
}
