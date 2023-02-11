#include <CatchUtils.h>
#include <chowdsp_gui/chowdsp_gui.h>

#define CHECK_OPENGL_CONTEXT_TESTS JUCE_MODULE_AVAILABLE_juce_opengl

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

static void attachToComponentTest (bool detach)
{
    chowdsp::OpenGLHelper openGlHelper;

    auto testComp = getTestComponent();
    openGlHelper.setComponent (testComp.get());
    REQUIRE_MESSAGE (openGlHelper.getComponent() == testComp.get(), "Attached component is incorrect!");

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
    REQUIRE_MESSAGE (actualContext == nullptr, "Component OpenGL context should be nullptr!");
#endif

    if (detach)
        REQUIRE_MESSAGE (openGlHelper.getComponent() == testComp.get(), "Attached component is incorrect!");
    else
        REQUIRE_MESSAGE (openGlHelper.getComponent() == nullptr, "Attached component should be nullptr!");
}

static void attachToNullTest (bool detach)
{
    chowdsp::OpenGLHelper openGlHelper;
    REQUIRE_MESSAGE (openGlHelper.getComponent() == nullptr, "Attached component should be nullptr!");

    openGlHelper.attach();
    REQUIRE_MESSAGE (! openGlHelper.isAttached(), "Should not be able to attach to null component!");

    if (detach)
        openGlHelper.detach();

    auto testComp = getTestComponent();
    openGlHelper.setComponent (testComp.get());

    if (detach)
        REQUIRE_MESSAGE (! openGlHelper.isAttached(), "Should NOT be attached!");
    else
        REQUIRE_MESSAGE (openGlHelper.isAttached(), "Should be attached!");
}

TEST_CASE ("OpenGL Helper Test", "[gui][opengl]")
{
    SECTION ("Attach To Component Test")
    {
        attachToComponentTest (true);
        attachToComponentTest (false);
    }

    SECTION ("Deleting Component Test")
    {
        chowdsp::OpenGLHelper openGlHelper;
        {
            auto testComp = getTestComponent();
            openGlHelper.setComponent (testComp.get());
            REQUIRE_MESSAGE (openGlHelper.getComponent() == testComp.get(), "Attached component is incorrect!");

            openGlHelper.attach();

#if CHECK_OPENGL_CONTEXT_TESTS
            auto* myContext = &openGlHelper.getOpenGLContext();
            auto* actualContext = OpenGLContext::getContextAttachedTo (*testComp);
            REQUIRE_MESSAGE (myContext == actualContext, "Incorrect OpenGLContext!");
#endif
        }

        REQUIRE_MESSAGE (openGlHelper.getComponent() == nullptr, "Attached component should be nullptr!");
    }

    SECTION ("Deleting Helper Test")
    {
        auto testComp = getTestComponent();
        {
            chowdsp::OpenGLHelper openGlHelper;
            openGlHelper.setComponent (testComp.get());
            REQUIRE_MESSAGE (openGlHelper.getComponent() == testComp.get(), "Attached component is incorrect!");

            openGlHelper.attach();

#if CHECK_OPENGL_CONTEXT_TESTS
            auto* myContext = &openGlHelper.getOpenGLContext();
            auto* actualContext = OpenGLContext::getContextAttachedTo (*testComp);
            REQUIRE_MESSAGE (myContext == actualContext, "Incorrect OpenGLContext!");
#endif
        }

#if CHECK_OPENGL_CONTEXT_TESTS
        auto* actualContext = OpenGLContext::getContextAttachedTo (*testComp);
        REQUIRE_MESSAGE (actualContext == nullptr, "Component OpenGL context should bt nullptr!");
#endif
    }

    SECTION ("Double Attach/Detach Test")
    {
        chowdsp::OpenGLHelper openGlHelper;

        auto testComp = getTestComponent();
        openGlHelper.setComponent (testComp.get());
        REQUIRE_MESSAGE (openGlHelper.getComponent() == testComp.get(), "Attached component is incorrect!");

        openGlHelper.attach();
        REQUIRE_MESSAGE (openGlHelper.isAttached(), "Should be attached!");

        openGlHelper.attach();
        REQUIRE_MESSAGE (openGlHelper.isAttached(), "Should still be attached!");

        openGlHelper.detach();
        REQUIRE_MESSAGE (! openGlHelper.isAttached(), "Should be detached!");

        openGlHelper.detach();
        REQUIRE_MESSAGE (! openGlHelper.isAttached(), "Should still be detached!");
    }

    SECTION ("Attach To New Component Test")
    {
        chowdsp::OpenGLHelper openGlHelper;

        auto testComp = getTestComponent();
        openGlHelper.setComponent (testComp.get());
        REQUIRE_MESSAGE (openGlHelper.getComponent() == testComp.get(), "Attached component is incorrect!");

        openGlHelper.attach();
        REQUIRE_MESSAGE (openGlHelper.isAttached(), "Should be attached!");

        auto testComp2 = getTestComponent();
        openGlHelper.setComponent (testComp2.get());
        REQUIRE_MESSAGE (openGlHelper.isAttached(), "Should still be attached!");
    }

    SECTION ("Attach To Null Test")
    {
        attachToNullTest (true);
        attachToNullTest (false);
    }

    SECTION ("Check OpenGL Available Test")
    {
        chowdsp::OpenGLHelper openGlHelper;
        auto isOpenGLAvailable = openGlHelper.isOpenGLAvailable();

#if JUCE_MODULE_AVAILABLE_juce_opengl
        REQUIRE_MESSAGE (isOpenGLAvailable, "OpenGL should be available on this platform!");
#else
        REQUIRE_MESSAGE (! isOpenGLAvailable, "OpenGL should not be available on this platform!");
#endif
    }
}
