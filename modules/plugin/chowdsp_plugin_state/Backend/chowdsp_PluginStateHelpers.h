#pragma once

namespace chowdsp
{
#ifndef DOXYGEN
namespace PluginStateHelpers
{
    template <typename ParamStateType, int count = 0, bool only_params = true, int index = pfr::tuple_size_v<ParamStateType>>
    struct ParamInfoHelper
    {
        template <typename T>
        static constexpr bool is_pfr_able = std::is_aggregate_v<T> && ! std::is_polymorphic_v<T>;

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
} // namespace PluginStateHelpers
#endif
} // namespace chowdsp
