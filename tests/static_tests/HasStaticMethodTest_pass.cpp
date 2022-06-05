#include <juce_core/juce_core.h>
#include <chowdsp_core/chowdsp_core.h>

struct Test
{
    static void staticMethod() {}
    static void oneArgStaticMethod(int) {}
    static void twoArgStaticMethod(int, float) {}
    static int nonVoidStaticMethod() { return 0; }
    static int oneArgNonVoidStaticMethod(int) { return 0; }
    static int twoArgNonVoidStaticMethod(int, float) { return 0; }

    CHOWDSP_CHECK_HAS_STATIC_METHOD(HasStaticMethod, Test, staticMethod);
    CHOWDSP_CHECK_HAS_STATIC_METHOD(HasStaticMethod2, Test, staticMethod2);
    static_assert(HasStaticMethod, "staticMethod should exist!");
    static_assert(! HasStaticMethod2, "staticMethod2 should not exist!");

    CHOWDSP_CHECK_HAS_STATIC_METHOD(HasOneArgStaticMethod, Test, oneArgStaticMethod);
    CHOWDSP_CHECK_HAS_STATIC_METHOD(HasOneArgStaticMethod2, Test, oneArgStaticMethod2);
    static_assert(HasOneArgStaticMethod, "should exist!");
    static_assert(! HasOneArgStaticMethod2, "should not exist!");

    CHOWDSP_CHECK_HAS_STATIC_METHOD(HasTwoArgStaticMethod, Test, twoArgStaticMethod);
    CHOWDSP_CHECK_HAS_STATIC_METHOD(HasTwoArgStaticMethod2, Test, twoArgStaticMethod2);
    static_assert(HasTwoArgStaticMethod, "should exist!");
    static_assert(! HasTwoArgStaticMethod2, "should not exist!");

    CHOWDSP_CHECK_HAS_STATIC_METHOD(HasNonVoidStaticMethod, Test, nonVoidStaticMethod);
    CHOWDSP_CHECK_HAS_STATIC_METHOD(HasNonVoidStaticMethod2, Test, nonVoidStaticMethod2);
    static_assert(HasNonVoidStaticMethod, "should exist!");
    static_assert(! HasNonVoidStaticMethod2, "should not exist!");

    CHOWDSP_CHECK_HAS_STATIC_METHOD(HasOneArgNonVoidStaticMethod, Test, oneArgNonVoidStaticMethod);
    CHOWDSP_CHECK_HAS_STATIC_METHOD(HasOneArgNonVoidStaticMethod2, Test, oneArgNonVoidStaticMethod2);
    static_assert(HasOneArgNonVoidStaticMethod, "should exist!");
    static_assert(! HasOneArgNonVoidStaticMethod2, "should not exist!");

    CHOWDSP_CHECK_HAS_STATIC_METHOD(HasTwoArgNonVoidStaticMethod, Test, twoArgNonVoidStaticMethod);
    CHOWDSP_CHECK_HAS_STATIC_METHOD(HasTwoArgNonVoidStaticMethod2, Test, twoArgNonVoidStaticMethod2);
    static_assert(HasTwoArgNonVoidStaticMethod, "should exist!");
    static_assert(! HasTwoArgNonVoidStaticMethod2, "should not exist!");
};

int main()
{
    Test t {};

    return 0;
}