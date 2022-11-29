#pragma once

namespace chowdsp
{
struct NullState
{
};

template <typename T>
struct StateValue
{
    using element_type = T;

    StateValue (std::string_view valueName, T defaultValue)
        : name (valueName),
          defaultVal (defaultValue),
          val (defaultVal)
    {
    }

    T get() const noexcept { return val; }
    operator T() const noexcept { return get(); }

    void set (T v)
    {
        if (v == val)
            return;

        val = v;
        changeBroadcaster();
    }

    void operator= (T v) { set (v); }

    void reset() { set (defaultVal); }

    const std::string_view name;
    const T defaultVal;

private:
    T val;
    chowdsp::Broadcaster<void()> changeBroadcaster;

    template <typename ParameterState, typename NonParameterState, typename Serializer>
    friend class PluginState;
};

#ifndef DOXYGEN
/** This API is unstable and should not be used directly! */
namespace PluginStateHelpers
{
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant")

    template <typename T>
    static constexpr bool is_pfr_able = std::is_aggregate_v<T> && ! std::is_polymorphic_v<T>;

    template <class T>
    struct IsStateValueType : std::false_type
    {
    };

    template <class T>
    struct IsStateValueType<StateValue<T>> : std::true_type
    {
    };

    template <typename T>
    static constexpr bool IsStateValue = IsStateValueType<T>::value;

    template <typename ParamStateType, int count = 0, bool only_params = true, int index = pfr::tuple_size_v<ParamStateType>>
    struct ParamInfoHelper
    {
        template <typename T, bool isParam, typename = void>
        struct SingleParamOrObjectInfo;

        template <typename T>
        struct SingleParamOrObjectInfo<T, true>
        {
            static constexpr int num_params = 1;
            static constexpr bool is_only_params = ParameterTypeHelpers::IsParameterPointerType<T>;
        };

        template <typename T>
        struct SingleParamOrObjectInfo<T, false, std::enable_if_t<is_pfr_able<T>>>
        {
            CHOWDSP_CHECK_HAS_STATIC_MEMBER (HasName, name)
            static_assert (HasName<T>, "Internal parameter structs must contain a static `name` member!");
            static_assert (std::is_same_v<decltype (T::name), const std::string_view>, "Internal parameter struct name must be a constexpr std::string_view!");

            static constexpr int num_params = ParamInfoHelper<T>::num_params;
            static constexpr bool is_only_params = ParamInfoHelper<T>::is_only_params;
        };

        template <typename T>
        struct SingleParamOrObjectInfo<T, false, std::enable_if_t<! is_pfr_able<T>>>
        {
            static constexpr int num_params = 0;
            static constexpr bool is_only_params = false;
        };

        using indexed_element_type = decltype (pfr::get<index - 1> (ParamStateType {}));
        static constexpr auto isParam = ParameterTypeHelpers::IsParameterPointerType<indexed_element_type>;

        static constexpr auto nextCount = count + SingleParamOrObjectInfo<indexed_element_type, isParam>::num_params;
        static constexpr int num_params = ParamInfoHelper<ParamStateType, nextCount, only_params, index - 1>::num_params;

        static constexpr auto nextOnlyParams = only_params & SingleParamOrObjectInfo<indexed_element_type, isParam>::is_only_params;
        static constexpr bool is_only_params = ParamInfoHelper<ParamStateType, count, nextOnlyParams, index - 1>::is_only_params;
    };

    template <typename ParamStateType, int count, bool only_params>
    struct ParamInfoHelper<ParamStateType, count, only_params, 0>
    {
        static constexpr int num_params = count;
        static constexpr bool is_only_params = only_params;
    };

    template <typename ParamStateType>
    static constexpr int ParamCount = ParamInfoHelper<ParamStateType>::num_params;

    template <typename ParamStateType>
    static constexpr int ContainsOnlyParamPointers = ParamInfoHelper<ParamStateType>::is_only_params;

    template <typename NonParamStateType, int count = 0, bool only_state_vals = true, int index = pfr::tuple_size_v<NonParamStateType>>
    struct NonParamInfoHelper
    {
        template <typename T, bool isStateVal, typename = void>
        struct SingleValOrObjectInfo;

        template <typename T>
        struct SingleValOrObjectInfo<T, true>
        {
            static constexpr int num_fields = 1;
            static constexpr bool is_only_state_vals = true;
        };

        template <typename T>
        struct SingleValOrObjectInfo<T, false, std::enable_if_t<is_pfr_able<T>>>
        {
            CHOWDSP_CHECK_HAS_STATIC_MEMBER (HasName, name)
            static_assert (HasName<T>, "Internal non-parameter state structs must contain a static `name` member!");
            static_assert (std::is_same_v<decltype (T::name), const std::string_view>, "Internal non-parameter state struct name must be a constexpr std::string_view!");

            static constexpr int num_fields = NonParamInfoHelper<T>::num_fields;
            static constexpr bool is_only_state_vals = NonParamInfoHelper<T>::is_only_state_vals;
        };

        template <typename T>
        struct SingleValOrObjectInfo<T, false, std::enable_if_t<! is_pfr_able<T>>>
        {
            static constexpr int num_fields = 0;
            static constexpr bool is_only_state_vals = false;
        };

        using indexed_element_type = decltype (pfr::get<index - 1> (NonParamStateType {}));
        static constexpr auto isStateVal = IsStateValue<indexed_element_type>;

        static constexpr auto nextCount = count + SingleValOrObjectInfo<indexed_element_type, isStateVal>::num_fields;
        static constexpr int num_params = NonParamInfoHelper<NonParamStateType, nextCount, only_state_vals, index - 1>::num_fields;

        static constexpr auto nextOnlyVals = only_state_vals & SingleValOrObjectInfo<indexed_element_type, isStateVal>::is_only_state_vals;
        static constexpr bool is_only_state_vals = NonParamInfoHelper<NonParamStateType, count, nextOnlyVals, index - 1>::is_only_state_vals;
    };

    template <typename NonParamStateType, int count, bool only_state_vals>
    struct NonParamInfoHelper<NonParamStateType, count, only_state_vals, 0>
    {
        static constexpr int num_fields = count;
        static constexpr bool is_only_state_vals = only_state_vals;
    };

    template <typename NonParamStateType>
    static constexpr int NonParamCount = NonParamInfoHelper<NonParamStateType>::num_fields;

    template <typename NonParamStateType>
    static constexpr int ContainsOnlyStateValues = NonParamInfoHelper<NonParamStateType>::is_only_state_vals;

    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
} // namespace PluginStateHelpers
#endif
} // namespace chowdsp
