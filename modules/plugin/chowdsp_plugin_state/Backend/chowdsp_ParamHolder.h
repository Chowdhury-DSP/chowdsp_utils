#pragma once

namespace chowdsp
{
/** A base class for storing parameters that can go into a plugin state. */
class ParamHolder
{
public:
    /** Convenient alias for a Parameter ID */
    using PID = ParameterID;

    /**
     * Creates a ParamHolder. The user might want to name the ParamHolder,
     * or make it "non-owning" so it doesn't take ownership of the parameter
     * pointers.
     */
    explicit ParamHolder (const juce::String& name = {}, bool isOwning = true);

    ParamHolder (ParamHolder&&) noexcept = default;
    ParamHolder& operator= (ParamHolder&&) noexcept = default;

    /** Adds parameters to the ParamHolder. */
    template <typename ParamType, typename... OtherParams>
    std::enable_if_t<std::is_base_of_v<FloatParameter, ParamType>, void>
        add (OptionalPointer<ParamType>& floatParam, OtherParams&... others);

    /** Adds parameters to the ParamHolder. */
    template <typename ParamType, typename... OtherParams>
    std::enable_if_t<std::is_base_of_v<ChoiceParameter, ParamType>, void>
        add (OptionalPointer<ParamType>& choiceParam, OtherParams&... others);

    /** Adds parameters to the ParamHolder. */
    template <typename ParamType, typename... OtherParams>
    std::enable_if_t<std::is_base_of_v<BoolParameter, ParamType>, void>
        add (OptionalPointer<ParamType>& boolParam, OtherParams&... others);

    /** Adds parameters to the ParamHolder. */
    template <typename ParamType, typename... OtherParams>
    std::enable_if_t<std::is_base_of_v<FloatParameter, ParamType>, void>
        add (const OptionalPointer<ParamType>& floatParam, OtherParams&... others);

    /** Adds parameters to the ParamHolder. */
    template <typename ParamType, typename... OtherParams>
    std::enable_if_t<std::is_base_of_v<ChoiceParameter, ParamType>, void>
        add (const OptionalPointer<ParamType>& choiceParam, OtherParams&... others);

    /** Adds parameters to the ParamHolder. */
    template <typename ParamType, typename... OtherParams>
    std::enable_if_t<std::is_base_of_v<BoolParameter, ParamType>, void>
        add (const OptionalPointer<ParamType>& boolParam, OtherParams&... others);

    /** Adds parameters to the ParamHolder. */
    template <typename... OtherParams>
    void add (ParamHolder& paramHolder, OtherParams&... others);

    /** Adds parameters to the ParamHolder. */
    template <typename ParamContainerType, typename... OtherParams>
    std::enable_if_t<TypeTraits::IsIterable<ParamContainerType>, void>
        add (ParamContainerType& paramContainer, OtherParams&... others);

    /** Counts all the parameters held internally. */
    [[nodiscard]] int count() const noexcept;

    /** Clears any parameters currently in the ParamHolder. */
    void clear();

    /** Connects all the parameters to an AudioProcessor */
    void connectParametersToProcessor (juce::AudioProcessor& processor);

    /** Returns the paramHolder name */
    [[nodiscard]] juce::String getName() const noexcept { return name; }

    /** Internal use only! */
    template <typename ParamContainersCallable, typename ParamHolderCallable>
    void doForAllParameterContainers (ParamContainersCallable&& paramContainersCallable, ParamHolderCallable&& paramHolderCallable);

    /** Internal use only! */
    template <typename ParamContainersCallable, typename ParamHolderCallable>
    void doForAllParameterContainers (ParamContainersCallable&& paramContainersCallable, ParamHolderCallable&& paramHolderCallable) const;

    /**
     * Do some callable for all the stored parameters.
     * Callable must have the signature void(ParameterType&, size_t).
     */
    template <typename Callable>
    size_t doForAllParameters (Callable&& callable, size_t index = 0);

    /**
     * Do some callable for all the stored parameters.
     * Callable must have the signature void(ParameterType&, size_t).
     */
    template <typename Callable>
    size_t doForAllParameters (Callable&& callable, size_t index = 0) const;

    /** Custom serializer */
    template <typename Serializer>
    static typename Serializer::SerializedType serialize (const ParamHolder& paramHolder);

    /** Custom deserializer */
    template <typename Serializer>
    static void deserialize (typename Serializer::DeserializedType deserial, ParamHolder& paramHolder);

    /** Recursively applies version streaming to the parameters herein. */
    void applyVersionStreaming (const Version&);

    /** Assign this function to apply version streaming to your non-parameter state. */
    std::function<void (const Version&)> versionStreamingCallback = nullptr;

private:
    void add() const
    {
        // base case!
    }

    std::vector<OptionalPointer<FloatParameter>> floatParams;
    std::vector<OptionalPointer<ChoiceParameter>> choiceParams;
    std::vector<OptionalPointer<BoolParameter>> boolParams;
    std::vector<ParamHolder*> otherParams;

    using ParamPtrVariant = std::variant<FloatParameter*, ChoiceParameter*, BoolParameter*>;
    std::unordered_map<std::string, ParamPtrVariant> allParamsMap {};

    juce::String name;
    bool isOwning;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamHolder)
};
} // namespace chowdsp

#include "chowdsp_ParamHolder.cpp"
