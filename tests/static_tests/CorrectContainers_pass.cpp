#include <map>
#include <juce_core/juce_core.h>
#include <chowdsp_core/chowdsp_core.h>

int main()
{
    using namespace chowdsp::TypeTraits;

    struct Dummy
    {
        int x;
        float y;

        void begin() {}
    };

    // these are containers!
    static_assert (IsIterable<std::array<int, 2>>);
    static_assert (IsIterable<std::vector<std::string>>);
    static_assert (IsIterable<std::map<juce::String, float>>);
    static_assert (IsIterable<juce::OwnedArray<Dummy>>);
    static_assert (IsIterable<std::string>);
    static_assert (IsIterable<juce::String>);

    // these are NOT containers!
    static_assert (! IsIterable<int>);
    static_assert (! IsIterable<Dummy>);

    // these are map-like
    static_assert (IsMapLike<std::map<int, std::string>>);
    static_assert (IsMapLike<std::unordered_map<juce::String, Dummy>>);

    // these are not map-like
    static_assert (! IsMapLike<Dummy>);
    static_assert (! IsMapLike<std::vector<int>>);

    return 0;
}
