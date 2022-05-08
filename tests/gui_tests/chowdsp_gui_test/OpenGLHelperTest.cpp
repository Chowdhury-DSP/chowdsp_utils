#include <TimedUnitTest.h>
#include <chowdsp_gui/chowdsp_gui.h>

#define CHECK_OPENGL_CONTEXT_TESTS JUCE_MODULE_AVAILABLE_juce_opengl && ! JUCE_LINUX

namespace
{
std::unique_ptr<juce::Component> getTestComponent()
{
    auto testComp = std::make_unique<juce::Component>();
    testComp->setBounds (0, 0, 100, 100);
    testComp->setVisible (true);

#if ! JUCE_LINUX // Linux in CI pipeline doesn't let you do this
    testComp->addToDesktop (juce::ComponentPeer::windowIsTemporary);
#endif

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpessimizing-move") // Clang doesn't like std::move
    return std::move (testComp);
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}
} // namespace

class OpenGLHelperTest : public TimedUnitTest
{
public:
    OpenGLHelperTest() : TimedUnitTest ("OpenGL Helper Test")
    {
    }

    void attachToComponentTest (bool detach)
    {
        chowdsp::OpenGLHelper openGlHelper;

        auto testComp = getTestComponent();
        openGlHelper.setComponent (testComp.get());
        expect (openGlHelper.getComponent() == testComp.get(), "Attached component is incorrect!");

        openGlHelper.attach();

#if CHECK_OPENGL_CONTEXT_TESTS
        auto* myContext = &openGlHelper.getOpenGLContext();
        auto* actualContext = OpenGLContext::getContextAttachedTo (*testComp);
        expect (myContext == actualContext, "Incorrect OpenGLContext!");
#endif

        if (detach)
            openGlHelper.detach();
        else
            openGlHelper.setComponent (nullptr);

#if CHECK_OPENGL_CONTEXT_TESTS
        actualContext = OpenGLContext::getContextAttachedTo (*testComp);
        expect (actualContext == nullptr, "Component OpenGL context should be nullptr!");
#endif

        if (detach)
            expect (openGlHelper.getComponent() == testComp.get(), "Attached component is incorrect!");
        else
            expect (openGlHelper.getComponent() == nullptr, "Attached component should be nullptr!");
    }

    void deletingComponentTest()
    {
        chowdsp::OpenGLHelper openGlHelper;
        {
            auto testComp = getTestComponent();
            openGlHelper.setComponent (testComp.get());
            expect (openGlHelper.getComponent() == testComp.get(), "Attached component is incorrect!");

            openGlHelper.attach();

#if CHECK_OPENGL_CONTEXT_TESTS
            auto* myContext = &openGlHelper.getOpenGLContext();
            auto* actualContext = OpenGLContext::getContextAttachedTo (*testComp);
            expect (myContext == actualContext, "Incorrect OpenGLContext!");
#endif
        }

        expect (openGlHelper.getComponent() == nullptr, "Attached component should be nullptr!");
    }

    void deletingHelperTest()
    {
        auto testComp = getTestComponent();
        {
            chowdsp::OpenGLHelper openGlHelper;
            openGlHelper.setComponent (testComp.get());
            expect (openGlHelper.getComponent() == testComp.get(), "Attached component is incorrect!");

            openGlHelper.attach();

#if CHECK_OPENGL_CONTEXT_TESTS
            auto* myContext = &openGlHelper.getOpenGLContext();
            auto* actualContext = OpenGLContext::getContextAttachedTo (*testComp);
            expect (myContext == actualContext, "Incorrect OpenGLContext!");
#endif
        }

#if CHECK_OPENGL_CONTEXT_TESTS
        auto* actualContext = OpenGLContext::getContextAttachedTo (*testComp);
        expect (actualContext == nullptr, "Component OpenGL context should bt nullptr!");
#endif
    }

    void doubleAttachTest()
    {
        chowdsp::OpenGLHelper openGlHelper;

        auto testComp = getTestComponent();
        openGlHelper.setComponent (testComp.get());
        expect (openGlHelper.getComponent() == testComp.get(), "Attached component is incorrect!");

        openGlHelper.attach();
        expect (openGlHelper.isAttached(), "Should be attached!");

        openGlHelper.attach();
        expect (openGlHelper.isAttached(), "Should still be attached!");

        openGlHelper.detach();
        expect (! openGlHelper.isAttached(), "Should be detached!");

        openGlHelper.detach();
        expect (! openGlHelper.isAttached(), "Should still be detached!");
    }

    void newAttachTest()
    {
        chowdsp::OpenGLHelper openGlHelper;

        auto testComp = getTestComponent();
        openGlHelper.setComponent (testComp.get());
        expect (openGlHelper.getComponent() == testComp.get(), "Attached component is incorrect!");

        openGlHelper.attach();
        expect (openGlHelper.isAttached(), "Should be attached!");

        auto testComp2 = getTestComponent();
        openGlHelper.setComponent (testComp2.get());
        expect (openGlHelper.isAttached(), "Should still be attached!");
    }

    void attachToNullTest (bool detach)
    {
        chowdsp::OpenGLHelper openGlHelper;
        expect (openGlHelper.getComponent() == nullptr, "Attached component should be nullptr!");

        openGlHelper.attach();
        expect (! openGlHelper.isAttached(), "Should not be able to attach to null component!");

        if (detach)
            openGlHelper.detach();

        auto testComp = getTestComponent();
        openGlHelper.setComponent (testComp.get());

        if (detach)
            expect (! openGlHelper.isAttached(), "Should NOT be attached!");
        else
            expect (openGlHelper.isAttached(), "Should be attached!");
    }

    void checkOpenGLAvailableTest()
    {
        chowdsp::OpenGLHelper openGlHelper;
        auto isOpenGLAvailable = openGlHelper.isOpenGLAvailable();

#if JUCE_MODULE_AVAILABLE_juce_opengl
        expect (isOpenGLAvailable, "OpenGL should be available on this platform!");
#else
        expect (! isOpenGLAvailable, "OpenGL should not be available on this platform!");
#endif
    }

    void runTestTimed() override
    {
        beginTest ("Attach To Component Test");
        attachToComponentTest (true);
        attachToComponentTest (false);

        beginTest ("Deleting Component Test");
        deletingComponentTest();

        beginTest ("Deleting Helper Test");
        deletingHelperTest();

        beginTest ("Double Attach/Detach Test");
        doubleAttachTest();

        beginTest ("Attach To New Component Test");
        newAttachTest();

        beginTest ("Attach To Null Test");
        attachToNullTest (true);
        attachToNullTest (false);

#if 0 // CI machines don't have up-to-date OpenGL, so let's skip this test for now
        beginTest ("Check OpenGL Available Test");
        checkOpenGLAvailableTest();
#endif
    }
};

#if ! JUCE_LINUX // can't run this test on Linux CI for some reason!
static OpenGLHelperTest openGlHelperTest;
#endif
