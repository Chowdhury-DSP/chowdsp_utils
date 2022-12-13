#pragma once

namespace chowdsp
{
/** A path to a parameter inside of a nested struct. */
struct ParameterPath
{
    constexpr ParameterPath (const std::string_view path) // NOSONAR NOLINT(google-explicit-constructor)
        : ParameterPath (path, delimiter (path))
    {
    }

    std::string_view head; // the path up to the first delimiter
    std::string_view tail; // the rest of the path starting from the first delimiter

    template <typename ParamType, typename ParamStateType>
    static constexpr ParamType* getParameterForPath (ParamStateType& params, const ParameterPath& path)
    {
        ParamType* paramPtr = nullptr;
        if (path.tail.empty())
        {
            pfr::for_each_field (params,
                                 [&path, &paramPtr] (auto& paramHolder)
                                 {
                                     using Type = std::decay_t<decltype (paramHolder)>;
                                     if constexpr (ParameterTypeHelpers::IsParameterPointerType<Type>)
                                     {
                                         if constexpr (std::is_base_of_v<ParamType, typename Type::element_type>)
                                         {
                                             if (paramHolder->paramID == toString (path.head))
                                                 paramPtr = static_cast<ParamType*> (paramHolder.get());
                                         }
                                     }
                                 });
        }
        else
        {
            pfr::for_each_field (params,
                                 [&path, &paramPtr] (auto& paramHolder)
                                 {
                                     using Type = std::decay_t<decltype (paramHolder)>;
                                     if constexpr (! ParameterTypeHelpers::IsParameterPointerType<Type>)
                                     {
                                         if (nameof::nameof_short_type<Type>() == path.head)
                                             paramPtr = getParameterForPath<ParamType> (paramHolder, path.tail);
                                     }
                                 });
        }

        return paramPtr;
    }

private:
    constexpr ParameterPath (const std::string_view path, size_t headLen)
        : ParameterPath (path, headLen, std::min (headLen + 1, path.size()))
    {
    }

    constexpr ParameterPath (const std::string_view path, size_t headLen, size_t tailStart)
        : head (path.substr (0, headLen)),
          tail (path.substr (tailStart, path.size() - tailStart))
    {
    }

    static constexpr size_t delimiter (const std::string_view sv)
    {
        return std::min (sv.find_first_of ('/'), sv.size());
    }
};
} // namespace chowdsp
