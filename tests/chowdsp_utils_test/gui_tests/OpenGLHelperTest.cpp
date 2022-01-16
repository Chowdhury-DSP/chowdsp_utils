#include <TimedUnitTest.h>

#if ! JUCE_LINUX // adding test component to desktop fails on Linux

namespace
{
std::unique_ptr<Component> getTestComponent()
{
    auto testComp = std::make_unique<Component>();
    testComp->setBounds (0, 0, 100, 100);
    testComp->setVisible (true);
    testComp->addToDesktop (ComponentPeer::windowIsTemporary);

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

    void attachToComponentTest()
    {
        chowdsp::OpenGLHelper openGlHelper;

        auto testComp = getTestComponent();
        openGlHelper.attachToComponent (testComp.get());

#if JUCE_MODULE_AVAILABLE_juce_opengl
        auto* myContext = &openGlHelper.getOpenGLContext();
        auto* actualContext = OpenGLContext::getContextAttachedTo (*testComp);
        expect (myContext == actualContext, "Incorrect OpenGLContext!");
#endif

        openGlHelper.attachToComponent (nullptr);

#if JUCE_MODULE_AVAILABLE_juce_opengl
        auto* actualContext = OpenGLContext::getContextAttachedTo (*testComp);
        expect (actualContext == nullptr, "Component OpenGL context should bt nullptr!");
#endif

        expect (openGlHelper.getAttachedComponent() == nullptr, "Attached component should be nullptr!");
    }

    void deletingComponentTest()
    {
        chowdsp::OpenGLHelper openGlHelper;
        {
            auto testComp = getTestComponent();
            openGlHelper.attachToComponent (testComp.get());

#if JUCE_MODULE_AVAILABLE_juce_opengl
            auto* myContext = &openGlHelper.getOpenGLContext();
            auto* actualContext = OpenGLContext::getContextAttachedTo (*testComp);
            expect (myContext == actualContext, "Incorrect OpenGLContext!");
#endif
        }

        expect (openGlHelper.getAttachedComponent() == nullptr, "Attached component should be nullptr!");
    }

    void deletingHelperTest()
    {
        auto testComp = getTestComponent();
        {
            chowdsp::OpenGLHelper openGlHelper;
            openGlHelper.attachToComponent (testComp.get());

#if JUCE_MODULE_AVAILABLE_juce_opengl
            auto* myContext = &openGlHelper.getOpenGLContext();
            auto* actualContext = OpenGLContext::getContextAttachedTo (*testComp);
            expect (myContext == actualContext, "Incorrect OpenGLContext!");
#endif
        }

#if JUCE_MODULE_AVAILABLE_juce_opengl
        auto* actualContext = OpenGLContext::getContextAttachedTo (*testComp);
        expect (actualContext == nullptr, "Component OpenGL context should bt nullptr!");
#endif
    }

    void runTestTimed() override
    {
        beginTest ("Attach To Component Test");
        attachToComponentTest();

        beginTest ("Deleting Component Test");
        deletingComponentTest();

        beginTest ("Deleting Helper Test");
        deletingHelperTest();
    }
};

static OpenGLHelperTest openGlHelperTest;

#endif // ! JUCE_LINUX
