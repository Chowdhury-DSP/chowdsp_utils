#include "chowdsp_OpenGLHelper.h"

// NOTE: some of this code was borrowed from
// SquarePine's HighPerformanceRenderConfigurator
// under the ISC license:
// https://github.com/SquarePine/squarepine_core/blob/main/modules/squarepine_graphics/components/HighPerformanceRendererConfigurator.cpp

#if CHOWDSP_OPENGL_IS_AVAILABLE
namespace
{
juce::String getGLString (GLenum value)
{
    if (juce::gl::glGetString != nullptr)
        return reinterpret_cast<const char*> (juce::gl::glGetString (value)); // NB: glGetString is from v2.0+.

    return juce::String {};
}

std::pair<int, int> getGLVersion()
{
    if (juce::gl::glGetIntegerv != nullptr)
    {
        GLint major = 0, minor = 0;
        juce::gl::glGetIntegerv (juce::gl::GL_MAJOR_VERSION, &major);
        juce::gl::glGetIntegerv (juce::gl::GL_MINOR_VERSION, &minor);
        return { (int) major, (int) minor };
    }

    return { 0, 0 };
}

auto createOpenGLTestComp (juce::OpenGLContext& ctx)
{
    struct TestComponent : juce::Component
    {
        juce::OpenGLContext& ctx;

        explicit TestComponent (juce::OpenGLContext& context) : ctx (context)
        {
            setSize (1, 1);
            setVisible (true);
            addToDesktop (juce::ComponentPeer::windowIsTemporary);
            ctx.attachTo (*this);
        }
        ~TestComponent() override { ctx.detach(); }
    };

    return std::make_unique<TestComponent> (ctx);
}

void checkOpenGLStats (juce::OpenGLContext& ctx, int& openGLMajorVersion, int& openGLMinorVersion)
{
    juce::Logger::writeToLog ("Attempting to check OpenGL stats...");
    auto testComp = createOpenGLTestComp (ctx);
    std::atomic_bool waiting { true };
    testComp->ctx.executeOnGLThread (
        [&waiting, &openGLMajorVersion, &openGLMinorVersion] (juce::OpenGLContext&)
        {
            std::tie (openGLMajorVersion, openGLMinorVersion) = getGLVersion();

            juce::String openGLStats;
            openGLStats
                << "=== OpenGL/GPU Information ===\n"
                << "Vendor: " << getGLString (juce::gl::GL_VENDOR) << "\n"
                << "Renderer: " << getGLString (juce::gl::GL_RENDERER) << "\n"
                << "OpenGL Version: " << getGLString (juce::gl::GL_VERSION) << "\n"
                << "OpenGL Major: " << juce::String (openGLMajorVersion) << "\n"
                << "OpenGL Minor: " << juce::String (openGLMinorVersion) << "\n"
                << "OpenGL Shading Language Version: " << getGLString (juce::gl::GL_SHADING_LANGUAGE_VERSION) << "\n";

            juce::Logger::writeToLog (openGLStats);
            waiting = false;
        },
        false);

    while (waiting)
#if JUCE_MODAL_LOOPS_PERMITTED
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
#else
        juce::Thread::sleep (100);
#endif
}
} // namespace
#endif

namespace chowdsp
{
OpenGLHelper::OpenGLHelper() // NOSONAR, NOLINT(modernize-use-equals-default): can only be default if compiling without OpenGL
{
#if CHOWDSP_OPENGL_IS_AVAILABLE
    checkOpenGLStats (openglContext, openGLMajorVersion, openGLMinorVersion);
#endif
}

OpenGLHelper::~OpenGLHelper()
{
    if (component != nullptr)
        componentBeingDeleted (*component);
}

bool OpenGLHelper::isOpenGLAvailable() const noexcept // NOLINT(readability-convert-member-functions-to-static): can only be static if compiling without OpenGL
{
#if CHOWDSP_OPENGL_IS_AVAILABLE
    return openGLMajorVersion >= 2; // For OpenGL drivers below v2.0, we get a black screen
#else
    return false;
#endif
}

void OpenGLHelper::attach()
{
    if (component == nullptr)
    {
        shouldAttachNextComponent = true;
        return;
    }

    shouldAttachNextComponent = false;

    if (attached)
        return;

    attached = true;

#if CHOWDSP_OPENGL_IS_AVAILABLE
    openglContext.attachTo (*component);
    component->addComponentListener (this);
#endif
}

void OpenGLHelper::detach()
{
    shouldAttachNextComponent = false;

    if (! attached)
        return;

    attached = false;

#if CHOWDSP_OPENGL_IS_AVAILABLE
    openglContext.detach();

    if (component != nullptr)
        component->removeComponentListener (this);
#endif
}

void OpenGLHelper::setComponent (juce::Component* newComp)
{
    bool wasAttached = attached;

    if (component != nullptr && wasAttached)
        detach();

    component = newComp;

    if (component != nullptr && (wasAttached || shouldAttachNextComponent))
        attach();
}

void OpenGLHelper::componentBeingDeleted (juce::Component& c)
{
    ignoreUnused (c);

    jassert (component == &c);

    if (attached)
        detach();
}
} // namespace chowdsp
