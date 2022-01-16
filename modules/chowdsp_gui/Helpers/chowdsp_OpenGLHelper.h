#pragma once

namespace chowdsp
{

/**
 * Helper class for attaching an OpenGL context to
 * a component. To use the class, call `attachToComponent()`
 * with the component that needs OpenGL. Then, if OpenGL
 * is available, the component will have a context attached
 * until it is destroyed, or until the attach method is
 * called again with a new component.
 *
 * If OpenGL is not available, the class can still be used,
 * but nothing will be happen.
 */
class OpenGLHelper : private juce::ComponentListener
{
public:
    /** Default constructor */
    OpenGLHelper() = default;

    /** Destructor */
    ~OpenGLHelper() override;

    /** Returns true if OpenGL is available/ */
    static constexpr bool isOpenGLAvailable()
    {
#if JUCE_MODULE_AVAILABLE_juce_opengl
        return true;
#else
        return false;
#endif
    }

    /**
     * Use this method to attach the OpenGL to a given component,
     * or call with nullptr to remove the OpenGL context from the
     * previous component.
     */
    void attach();

    void detach();

    bool isAttached() const noexcept { return attached; }

    void setComponent (juce::Component* component);

    /** Returns the currently attached component, or nullptr. */
    juce::Component* getComponent() const noexcept { return component; }

#if JUCE_MODULE_AVAILABLE_juce_opengl
    /** Returns the OpenGL context. */
    juce::OpenGLContext& getOpenGLContext() { return openglContext; }
#endif

private:
    void componentBeingDeleted (juce::Component& component) final;

    juce::Component* component = nullptr;
    bool attached = false;

#if JUCE_MODULE_AVAILABLE_juce_opengl
    juce::OpenGLContext openglContext;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLHelper)
};
} // namespace chowdsp
