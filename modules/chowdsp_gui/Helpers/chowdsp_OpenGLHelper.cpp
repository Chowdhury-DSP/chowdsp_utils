#include "chowdsp_OpenGLHelper.h"

namespace chowdsp
{
OpenGLHelper::~OpenGLHelper()
{
    attachToComponent (nullptr);
}

void OpenGLHelper::attachToComponent (juce::Component* component)
{
    if (component == getAttachedComponent())
        return; // no change!

#if JUCE_MODULE_AVAILABLE_juce_opengl
    // remove if currently attached to a component
    if (auto* oldComp = openglContext.getTargetComponent())
    {
        openglContext.detach();
        oldComp->removeComponentListener (this);
    }

    if (component)
    {
        openglContext.attachTo (*component);
        component->addComponentListener (this);
    }
#endif
}

juce::Component* OpenGLHelper::getAttachedComponent()
{
#if JUCE_MODULE_AVAILABLE_juce_opengl
    return openglContext.getTargetComponent();
#else
    return nullptr;
#endif
}

void OpenGLHelper::componentBeingDeleted (juce::Component&)
{
    attachToComponent (nullptr);
}
} // namespace chowdsp
