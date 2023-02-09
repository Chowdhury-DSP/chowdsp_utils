#include <CatchUtils.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>
#include <chowdsp_gui/chowdsp_gui.h>

TEST_CASE ("LNF Allocator Test", "[plugin][utilities]")
{
    SECTION ("Get LookAndFeel Test")
    {
        chowdsp::LNFAllocator allocator;
        auto* lnf = allocator.getLookAndFeel<chowdsp::ChowLNF>();
        auto* lnf2 = allocator.getLookAndFeel<chowdsp::ChowLNF>();

        REQUIRE_MESSAGE (lnf == lnf2, "Look and feel pointers should be the same!");
    }

    SECTION ("Add LookAndFeel Test")
    {
        chowdsp::LNFAllocator allocator;
        auto* lnf = allocator.addLookAndFeel<chowdsp::ChowLNF>();
        auto* lnf2 = allocator.getLookAndFeel<chowdsp::ChowLNF>();
        auto* lnf3 = allocator.addLookAndFeel<chowdsp::ChowLNF>();

        REQUIRE_MESSAGE (lnf == lnf2, "Look and feel pointers should be the same!");
        REQUIRE_MESSAGE (lnf == lnf3, "Look and feel pointers should be the same!");
    }

    SECTION ("Contains LookAndFeel Test")
    {
        chowdsp::LNFAllocator allocator;
        allocator.getLookAndFeel<chowdsp::ChowLNF>();
        REQUIRE_MESSAGE (allocator.containsLookAndFeelType<chowdsp::ChowLNF>(), "Allocator should contain this look and feel!");
        REQUIRE_MESSAGE (! allocator.containsLookAndFeelType<juce::LookAndFeel_V4>(), "Allocator should not contain this look and feel!");
    }
}
