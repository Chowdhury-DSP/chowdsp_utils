#include "chowdsp_OpenGLHelper.h"

namespace chowdsp
{
OpenGLHelper::~OpenGLHelper()
{
    componentBeingDeleted (*component);
}

void OpenGLHelper::attach()
{
#if JUCE_MODULE_AVAILABLE_juce_opengl
    if (component == nullptr)
    {
        // make sure to set the component before trying to attach!
        jassertfalse;
        return;
    }

    if (attached)
        return;

    attached = true;
    openglContext.attachTo (*component);
#endif
}

void OpenGLHelper::detach()
{
#if JUCE_MODULE_AVAILABLE_juce_opengl
    if (! attached)
        return;

    attached = false;
    openglContext.detach();
#endif
}

void OpenGLHelper::setComponent (juce::Component* newComp)
{
    if (component != nullptr)
    {
        component->removeComponentListener (this);
        if (attached)
            detach();
    }

    component = newComp;

    if (component != nullptr)
    {
        component->addComponentListener (this);
        if (attached)
            attach();
    }
}

void OpenGLHelper::componentBeingDeleted (juce::Component& c)
{
    jassert (component == &c);

    component->removeComponentListener (this);
    if (attached)
        detach();
}
} // namespace chowdsp
