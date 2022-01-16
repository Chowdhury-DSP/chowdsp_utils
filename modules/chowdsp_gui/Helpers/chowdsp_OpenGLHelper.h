#pragma once

namespace chowdsp
{
class OpenGLHelper : private juce::ComponentListener
{
public:
    OpenGLHelper() = default;
    ~OpenGLHelper() override;

    static constexpr bool isOpenGLAvailable()
    {
#if JUCE_MODULE_AVAILABLE_juce_opengl
        return true;
#else
        return false;
#endif
    }

    void attachToComponent (juce::Component* component);

    juce::Component* getAttachedComponent();

#if JUCE_MODULE_AVAILABLE_juce_opengl
    juce::OpenGLContext& getOpenGLContext()
    {
        return openglContext;
    }
#endif

private:
    void componentBeingDeleted (juce::Component& component) override;

#if JUCE_MODULE_AVAILABLE_juce_opengl
    juce::OpenGLContext openglContext;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLHelper)
};
} // namespace chowdsp
