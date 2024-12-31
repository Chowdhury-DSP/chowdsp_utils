#pragma once

namespace chowdsp
{
/** Helper methods for working with enums */
namespace EnumHelpers
{
#if CHOWDSP_USING_JUCE
    /**
     * Creates a juce::StringArray from the values of an enum class.
     *
     * If you need to replace characters from your enum, use the charMap argument to do so.
     * For example, `createStringArray<MyEnum> ({ { '_', ' ' } }), will replace the underscores
     * in your enum names with spaces.
     */
    template <typename EnumType>
    juce::StringArray createStringArray (const std::initializer_list<std::pair<char, char>>& charMap = {})
    {
        juce::StringArray names;
        for (auto& name : magic_enum::enum_names<EnumType>())
        {
            juce::String nameStr { name.data() };
            for (const auto& [charToReplace, charToReplaceWith] : charMap)
                nameStr = nameStr.replaceCharacter (charToReplace, charToReplaceWith).trim();

            names.add (nameStr);
        }

        return names;
    }
#endif
} // namespace EnumHelpers
} // namespace chowdsp
