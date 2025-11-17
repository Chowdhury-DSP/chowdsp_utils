#pragma once

// LCOV_EXCL_START

#if JUCE_MODULE_AVAILABLE_juce_graphics
#include <juce_graphics/juce_graphics.h>

namespace chowdsp
{
/** Clone of juce::Rectangle but with everything constexpr'd */
template <typename T>
struct Rectangle
{
    static_assert (std::is_arithmetic_v<T>, "Rectangle type must templated on an arithmetic type!");

    /** The type on which this point is templated */
    using value_type = T;

    juce::Point<T> position {};
    T width {};
    T height {};

    constexpr Rectangle() = default;
    constexpr Rectangle (const Rectangle&) = default;
    constexpr Rectangle& operator= (const Rectangle&) = default;
    constexpr Rectangle (Rectangle&&) noexcept = default;
    constexpr Rectangle& operator= (Rectangle&&) noexcept = default;

    constexpr Rectangle (T x, T y, T initialWidth, T initialHeight) : position (x, y),
                                                                      width (initialWidth),
                                                                      height (initialHeight)
    {
    }

    constexpr Rectangle (T initialWidth, T initialHeight) : width (initialWidth),
                                                            height (initialHeight)
    {
    }

    [[nodiscard]] constexpr bool isEmpty() const noexcept
    {
        return width == T {} || height == T {};
    }

    [[nodiscard]] constexpr T getX() const noexcept
    {
        return position.x;
    }

    [[nodiscard]] constexpr T getY() const noexcept
    {
        return position.y;
    }

    [[nodiscard]] constexpr T getWidth() const noexcept
    {
        return width;
    }

    [[nodiscard]] constexpr T getHeight() const noexcept
    {
        return height;
    }

    [[nodiscard]] constexpr T getRight() const noexcept
    {
        return getX() + getWidth();
    }

    [[nodiscard]] constexpr T getBottom() const noexcept
    {
        return getY() + getHeight();
    }

    [[nodiscard]] constexpr T getCentreX() const noexcept
    {
        return getX() + getWidth() / (T) 2;
    }

    [[nodiscard]] constexpr T getCentreY() const noexcept
    {
        return getY() + getHeight() / (T) 2;
    }

    [[nodiscard]] constexpr juce::Point<T> getCentre() const noexcept
    {
        return { getCentreX(), getCentreY() };
    }

    [[nodiscard]] constexpr juce::Point<T> getPosition() const noexcept
    {
        return position;
    }

    constexpr void setPosition (juce::Point<T> newPos) noexcept
    {
        position = newPos;
    }

    constexpr void setPosition (T newX, T newY) noexcept
    {
        setPosition ({ newX, newY });
    }

    [[nodiscard]] constexpr juce::Point<T> getTopLeft() const noexcept
    {
        return position;
    }

    [[nodiscard]] constexpr juce::Point<T> getTopRight() const noexcept
    {
        return { position.x + width, position.y };
    }

    [[nodiscard]] constexpr juce::Point<T> getBottomLeft() const noexcept
    {
        return { position.x, position.y + height };
    }

    [[nodiscard]] constexpr juce::Point<T> getBottomRight() const noexcept
    {
        return { position.x + width, position.y + height };
    }

    constexpr void setSize (T newWidth, T newHeight) noexcept
    {
        width = newWidth;
        height = newHeight;
    }

    constexpr void setBounds (T newX, T newY, T newWidth, T newHeight) noexcept
    {
        position.x = newX;
        position.y = newY;
        width = newWidth;
        height = newHeight;
    }

    constexpr void setX (T newX) noexcept
    {
        position.x = newX;
    }

    constexpr void setY (T newY) noexcept
    {
        position.y = newY;
    }

    constexpr void setWidth (T newWidth) noexcept
    {
        width = newWidth;
    }

    constexpr void setHeight (T newHeight) noexcept
    {
        height = newHeight;
    }

    constexpr void setCentre (T newCentreX, T newCentreY) noexcept
    {
        position.x = newCentreX - width / (T) 2;
        position.y = newCentreY - height / (T) 2;
    }

    constexpr void setCentre (juce::Point<T> newCentre) noexcept
    {
        setCentre (newCentre.x, newCentre.y);
    }

    [[nodiscard]] constexpr Rectangle withX (T newX) const noexcept
    {
        return { newX, position.y, width, height };
    }

    [[nodiscard]] constexpr Rectangle withY (T newY) const noexcept
    {
        return { position.x, newY, width, height };
    }

    [[nodiscard]] constexpr Rectangle withRightX (T newRightX) const noexcept
    {
        return { newRightX - width, position.y, width, height };
    }

    [[nodiscard]] constexpr Rectangle withBottomY (T newBottomY) const noexcept
    {
        return { position.x, newBottomY - height, width, height };
    }

    [[nodiscard]] constexpr Rectangle withPosition (T newX, T newY) const noexcept
    {
        return { newX, newY, width, height };
    }

    [[nodiscard]] constexpr Rectangle withPosition (juce::Point<T> newPos) const noexcept
    {
        return { newPos.x, newPos.y, width, height };
    }

    [[nodiscard]] constexpr Rectangle withZeroOrigin() const noexcept
    {
        return { width, height };
    }

    [[nodiscard]] constexpr Rectangle withCentre (juce::Point<T> newCentre) const noexcept
    {
        return { newCentre.x - width / (T) 2,
                 newCentre.y - height / (T) 2,
                 width,
                 height };
    }

    [[nodiscard]] constexpr Rectangle withWidth (T newWidth) const noexcept
    {
        return { position.x, position.y, juce::jmax (T(), newWidth), height };
    }

    [[nodiscard]] constexpr Rectangle withHeight (T newHeight) const noexcept
    {
        return { position.x, position.y, width, juce::jmax (T(), newHeight) };
    }

    [[nodiscard]] constexpr Rectangle withSize (T newWidth, T newHeight) const noexcept
    {
        return { position.x, position.y, juce::jmax (T(), newWidth), juce::jmax (T(), newHeight) };
    }

    [[nodiscard]] constexpr Rectangle withSizeKeepingCentre (T newWidth, T newHeight) const noexcept
    {
        return { position.x + (width - newWidth) / (T) 2,
                 position.y + (height - newHeight) / (T) 2,
                 newWidth,
                 newHeight };
    }

    constexpr void setLeft (T newLeft) noexcept
    {
        width = juce::jmax (T(), position.x + width - newLeft);
        position.x = newLeft;
    }

    [[nodiscard]] constexpr Rectangle withLeft (T newLeft) const noexcept
    {
        return { newLeft,
                 position.y,
                 juce::jmax (T(), position.x + width - newLeft),
                 height };
    }

    constexpr void setTop (T newTop) noexcept
    {
        height = juce::jmax (T(), position.y + height - newTop);
        position.y = newTop;
    }

    [[nodiscard]] constexpr Rectangle withTop (T newTop) const noexcept
    {
        return { position.x,
                 newTop,
                 width,
                 juce::jmax (T(), position.y + height - newTop) };
    }

    constexpr void setRight (T newRight) noexcept
    {
        position.x = juce::jmin (position.x, newRight);
        width = newRight - position.x;
    }

    [[nodiscard]] constexpr Rectangle withRight (T newRight) const noexcept
    {
        return { juce::jmin (position.x, newRight),
                 position.y,
                 juce::jmax (T(), newRight - position.x),
                 height };
    }

    constexpr void setBottom (T newBottom) noexcept
    {
        position.y = juce::jmin (position.y, newBottom);
        height = newBottom - position.y;
    }

    [[nodiscard]] constexpr Rectangle withBottom (T newBottom) const noexcept
    {
        return { position.x,
                 juce::jmin (position.y, newBottom),
                 width,
                 juce::jmax (T(), newBottom - position.y) };
    }

    [[nodiscard]] constexpr Rectangle withTrimmedLeft (T amountToRemove) const noexcept
    {
        return withLeft (position.x + amountToRemove);
    }

    [[nodiscard]] constexpr Rectangle withTrimmedRight (T amountToRemove) const noexcept
    {
        return withWidth (width - amountToRemove);
    }

    [[nodiscard]] constexpr Rectangle withTrimmedTop (T amountToRemove) const noexcept
    {
        return withTop (position.y + amountToRemove);
    }

    [[nodiscard]] constexpr Rectangle withTrimmedBottom (T amountToRemove) const noexcept
    {
        return withHeight (height - amountToRemove);
    }

    constexpr void translate (T deltaX, T deltaY) noexcept
    {
        position.x += deltaX;
        position.y += deltaY;
    }

    [[nodiscard]] constexpr Rectangle translated (T deltaX, T deltaY) const noexcept
    {
        return { position.x + deltaX, position.y + deltaY, width, height };
    }

    constexpr void expand (T deltaX, T deltaY) noexcept
    {
        auto nw = juce::jmax (T(), width + deltaX * 2);
        auto nh = juce::jmax (T(), height + deltaY * 2);
        setBounds (position.x - deltaX, position.y - deltaY, nw, nh);
    }

    [[nodiscard]] constexpr Rectangle expanded (T deltaX, T deltaY) const noexcept
    {
        auto nw = juce::jmax (T(), width + deltaX * 2);
        auto nh = juce::jmax (T(), height + deltaY * 2);
        return { position.x - deltaX, position.y - deltaY, nw, nh };
    }

    [[nodiscard]] constexpr Rectangle expanded (T delta) const noexcept
    {
        return expanded (delta, delta);
    }

    constexpr void reduce (T deltaX, T deltaY) noexcept
    {
        expand (-deltaX, -deltaY);
    }

    [[nodiscard]] constexpr Rectangle reduced (T deltaX, T deltaY) const noexcept
    {
        return expanded (-deltaX, -deltaY);
    }

    [[nodiscard]] constexpr Rectangle reduced (T delta) const noexcept
    {
        return reduced (delta, delta);
    }

    [[nodiscard]] constexpr Rectangle removeFromTop (T amountToRemove) noexcept
    {
        const Rectangle r { position.x, position.y, width, juce::jmin (amountToRemove, height) };
        position.y += r.height;
        height -= r.height;
        return r;
    }

    [[nodiscard]] constexpr Rectangle removeFromLeft (T amountToRemove) noexcept
    {
        const Rectangle r { position.x, position.y, juce::jmin (amountToRemove, width), height };
        position.x += r.width;
        width -= r.width;
        return r;
    }

    [[nodiscard]] constexpr Rectangle removeFromRight (T amountToRemove) noexcept
    {
        amountToRemove = juce::jmin (amountToRemove, width);
        const Rectangle r { position.x + width - amountToRemove, position.y, amountToRemove, height };
        width -= amountToRemove;
        return r;
    }

    [[nodiscard]] constexpr Rectangle removeFromBottom (T amountToRemove) noexcept
    {
        amountToRemove = juce::jmin (amountToRemove, height);
        const Rectangle r { position.x, position.y + height - amountToRemove, width, amountToRemove };
        height -= amountToRemove;
        return r;
    }

    template <typename FloatType>
    [[nodiscard]] constexpr T proportionOfWidth (FloatType proportion) const noexcept
    {
        return static_cast<T> ((FloatType) width * proportion);
    }

    /** Returns a proportion of the height of this rectangle. */
    template <typename FloatType>
    [[nodiscard]] constexpr T proportionOfHeight (FloatType proportion) const noexcept
    {
        return static_cast<T> ((FloatType) height * proportion);
    }

    template <typename FloatType>
    [[nodiscard]] constexpr Rectangle getProportion (Rectangle<FloatType> proportionalRect) const noexcept
    {
        return { position.x + static_cast<T> (width * proportionalRect.pos.x),
                 position.y + static_cast<T> (height * proportionalRect.pos.y),
                 proportionOfWidth (proportionalRect.w),
                 proportionOfHeight (proportionalRect.h) };
    }

    [[nodiscard]] constexpr bool operator== (const Rectangle& other) const noexcept
    {
        return position == other.position && width == other.width && height == other.height;
    }

    [[nodiscard]] constexpr bool operator!= (const Rectangle& other) const noexcept
    {
        return position != other.position || width != other.width || height != other.height;
    }

    [[nodiscard]] constexpr Rectangle<int> toNearestInt() const noexcept
    {
        return { juce::roundToInt (position.x),
                 juce::roundToInt (position.y),
                 juce::roundToInt (width),
                 juce::roundToInt (height) };
    }

    [[nodiscard]] constexpr Rectangle<float> toFloat() const noexcept
    {
        return { static_cast<float> (position.x),
                 static_cast<float> (position.y),
                 static_cast<float> (width),
                 static_cast<float> (height) };
    }

    [[nodiscard]] constexpr Rectangle<double> toDouble() const noexcept
    {
        return { static_cast<double> (position.x),
                 static_cast<double> (position.y),
                 static_cast<double> (width),
                 static_cast<double> (height) };
    }

    [[nodiscard]] juce::Rectangle<T> toJuceRectangle() const noexcept
    {
        return juce::Rectangle { position.x,
                                 position.y,
                                 width,
                                 height };
    }

    operator juce::Rectangle<T>() const noexcept // NOSONAR, NOLINT(google-explicit-constructor): we want to be able to do implicit conversion here
    {
        return toJuceRectangle();
    }
};
} // namespace chowdsp

#endif // JUCE_MODULE_AVAILABLE_juce_graphics

// LCOV_EXCL_END
