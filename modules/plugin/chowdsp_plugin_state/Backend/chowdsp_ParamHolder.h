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
    explicit ParamHolder (ParamHolder* parent = nullptr, std::string_view name = {}, bool isOwning = true);
    explicit ParamHolder (ChainedArenaAllocator& alloc, std::string_view name = {}, bool isOwning = true);
    ~ParamHolder();

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

    /** Connects all the parameters to an AudioProcessor */
    void connectParametersToProcessor (juce::AudioProcessor& processor);

    /** Returns the paramHolder name */
    [[nodiscard]] std::string_view getName() const noexcept { return name; }

    /** Internal use only! */
    template <typename ParamCallable, typename ParamHolderCallable>
    void doForAllParametersOrContainers (ParamCallable&& paramCallable, ParamHolderCallable&& paramHolderCallable);

    /** Internal use only! */
    template <typename ParamCallable, typename ParamHolderCallable>
    void doForAllParametersOrContainers (ParamCallable&& paramCallable, ParamHolderCallable&& paramHolderCallable) const;

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

    /** Resets all parameters to their default values */
    void reset();

    /** Custom serializer */
    static json serialize_json (const ParamHolder& paramHolder);

    /** Custom deserializer */
    static void deserialize_json (const json& deserial, ParamHolder& paramHolder);

    /** Legacy deserializer */
    static void legacy_deserialize (const json& deserial, ParamHolder& paramHolder);

    /** Recursively applies version streaming to the parameters herein. */
    void applyVersionStreaming (const Version&);

    /** Assign this function to apply version streaming to your non-parameter state. */
    FixedSizeFunction<8, void (const Version&)> versionStreamingCallback {};

    OptionalPointer<ChainedArenaAllocator> arena {};

private:
    void add() const
    {
        // base case!
    }

    enum ThingInfo : uint8_t
    {
        FloatParam = 0,
        ChoiceParam = 1,
        BoolParam = 2,
        Holder = 3,

        ShouldDelete = 4,
    };
    using ThingPtr = PackedPointer<PackedVoid>;
    ChunkList<ThingPtr, 8> things { arena };

    static ThingInfo getType (const ThingPtr& thingPtr)
    {
        return static_cast<ThingInfo> (thingPtr.get_flags() & ~ShouldDelete);
    }
    static bool getShouldDelete (const ThingPtr& thingPtr)
    {
        return thingPtr.get_flags() & ShouldDelete;
    }
    static uint8_t getFlags (ThingInfo type, bool shouldDelete)
    {
        return static_cast<uint8_t> (type | (shouldDelete ? ShouldDelete : 0));
    }

    struct ParamDeserial
    {
        std::string_view id {};
        ThingPtr ptr {};
        bool found = false;
    };
    using ParamDeserialList = ChunkList<ParamDeserial, 100>;
    static void getParameterPointers (ParamHolder& holder, ParamDeserialList& parameters);

    std::string_view name;
    bool isOwning;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamHolder)
};
} // namespace chowdsp

#include "chowdsp_ParamHolder.cpp"
