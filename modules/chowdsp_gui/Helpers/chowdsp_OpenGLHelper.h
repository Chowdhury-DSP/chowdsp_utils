#pragma once

namespace chowdsp
{
/**
 * Helper class for attaching an OpenGL context to a component.
 *
 * If OpenGL is not available, the class can still be used
 * pretty much the same way, but nothing will happen.
 */
class OpenGLHelper : private juce::ComponentListener
{
public:
    /** Default constructor */
    OpenGLHelper();

    /** Destructor */
    ~OpenGLHelper() override;

    /** Returns true if OpenGL is available/ */
    [[nodiscard]] bool isOpenGLAvailable() const noexcept;

    /** Use this method to attach the OpenGL to the current component. */
    void attach();

    /**
     * Use this method to detach the OpenGL to the current component.
     * If no component is currently set, nothing will happen.
     */
    void detach();

    /** Returns true if the OpenGL context is currently attached. */
    [[nodiscard]] bool isAttached() const noexcept { return attached; }

    /** Sets the component to attach an OpenGL context to. */
    void setComponent (juce::Component* component);

    /** Returns the currently attached component, or nullptr. */
    [[nodiscard]] juce::Component* getComponent() const noexcept { return component.getComponent(); }

#if CHOWDSP_OPENGL_IS_AVAILABLE
    /** Returns the OpenGL context. */
    juce::OpenGLContext& getOpenGLContext() { return openglContext; }
#endif

private:
    void componentBeingDeleted (juce::Component& component) final;

    juce::Component::SafePointer<juce::Component> component;
    bool attached = false;

    bool shouldAttachNextComponent = false;

#if CHOWDSP_OPENGL_IS_AVAILABLE
    juce::OpenGLContext openglContext;

    int openGLMajorVersion = 0;
    int openGLMinorVersion = 0;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLHelper)
};
} // namespace chowdsp
