#include "chowdsp_OpenGLHelper.h"

namespace chowdsp
{
OpenGLHelper::~OpenGLHelper()
{
    if (component != nullptr)
        componentBeingDeleted (*component);
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
