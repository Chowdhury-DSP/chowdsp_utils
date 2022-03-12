#include "chowdsp_OpenGLHelper.h"

// NOTE: some of this code was borrowed from
// SquarePine's HighPerformanceRenderConfigurator
// under the ISC license:
// https://github.com/SquarePine/squarepine_core/blob/main/modules/squarepine_graphics/components/HighPerformanceRendererConfigurator.cpp

namespace
{
#if JUCE_MODULE_AVAILABLE_juce_opengl
juce::String getGLString (GLenum value)
{
    return reinterpret_cast<const char*> (juce::gl::glGetString (value)); // NB: glGetString is from v2.0+.
}

auto createOpenGLTestComp()
{
    struct TestComponent : juce::Component
    {
        juce::OpenGLContext ctx;

        TestComponent()
        {
            setSize (1, 1);
            setVisible (true);
            addToDesktop (juce::ComponentPeer::windowIsTemporary);
            ctx.attachTo (*this);
        }
        ~TestComponent() override
        {
            ctx.detach();
        }

        void paint (juce::Graphics&) override
        {
            std::cout << "PAINTING..." << std::endl;
        }
    };

    return std::make_unique<TestComponent>();
}
#endif

void logOpenGLStats()
{
#if JUCE_MODULE_AVAILABLE_juce_opengl
    auto testComp = createOpenGLTestComp();
    std::atomic_bool waiting { true };
    testComp->ctx.executeOnGLThread (
        [&waiting] (juce::OpenGLContext&)
        {
            GLint major = 0, minor = 0;
            juce::gl::glGetIntegerv (juce::gl::GL_MAJOR_VERSION, &major);
            juce::gl::glGetIntegerv (juce::gl::GL_MINOR_VERSION, &minor);

            juce::String openGLStats;
            openGLStats
                << "=== OpenGL/GPU Information ===\n"
                << "Vendor: " << getGLString (juce::gl::GL_VENDOR) << "\n"
                << "Renderer: " << getGLString (juce::gl::GL_RENDERER) << "\n"
                << "OpenGL Version: " << getGLString (juce::gl::GL_VERSION) << "\n"
                << "OpenGL Major: " << juce::String (major) << "\n"
                << "OpenGL Minor: " << juce::String (minor) << "\n"
                << "OpenGL Shading Language Version: " << getGLString (juce::gl::GL_SHADING_LANGUAGE_VERSION) << "\n";

            juce::Logger::writeToLog (openGLStats);
            waiting = false;
        },
        false);
    while (waiting)
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
    std::cout << "DONE" << std::endl;
#else
    juce::Logger::writeToLog ("JUCE: program was not compiled with OpenGL!");
#endif
}

} // namespace

namespace chowdsp
{
OpenGLHelper::OpenGLHelper()
{
    logOpenGLStats();
}

OpenGLHelper::~OpenGLHelper()
{
    if (component != nullptr)
        componentBeingDeleted (*component);
}

bool OpenGLHelper::isOpenGLAvailable()
{
#if JUCE_MODULE_AVAILABLE_juce_opengl
    logOpenGLStats();

    GLint major = 0;
    juce::gl::glGetIntegerv (juce::gl::GL_MAJOR_VERSION, &major);

    return major >= 2; // For OpenGL drivers below v2.0, we get a black screen
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

#if JUCE_MODULE_AVAILABLE_juce_opengl
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

#if JUCE_MODULE_AVAILABLE_juce_opengl
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
