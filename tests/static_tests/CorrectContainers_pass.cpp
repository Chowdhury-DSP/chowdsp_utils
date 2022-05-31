#include <map>
#include <juce_core/juce_core.h>
#include <chowdsp_core/chowdsp_core.h>

int main()
{
    using namespace chowdsp::MetaHelpers;

    struct Dummy
    {
        int x;
        float y;

        void begin() {}
    };

    // these are containers!
    static_assert (IsContainer<std::array<int, 2>>);
    static_assert (IsContainer<std::vector<std::string>>);
    static_assert (IsContainer<std::map<juce::String, float>>);
    static_assert (IsContainer<juce::OwnedArray<Dummy>>);
    static_assert (IsContainer<std::string>);
    static_assert (IsContainer<juce::String>);

    // these are NOT containers!
    static_assert (! IsContainer<int>);
    static_assert (! IsContainer<Dummy>);

    return 0;
}
