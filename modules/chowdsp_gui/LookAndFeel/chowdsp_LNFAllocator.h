#pragma once

namespace chowdsp
{
/** 
 * A global allocator to use for look and feels.
 * This class is best used as a Singleton, or SharedResourcePointer.
 */
class LNFAllocator
{
public:
    LNFAllocator()
    {
        lnfs.reserve (24);
    }

    ~LNFAllocator()
    {
        juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    }

    /**
     * Returns this type of look and feel.
     * If the allocator does not already contain this look and feel,
     * it will be created and added to the allocator.
     */
    template <typename LookAndFeelSubclass>
    juce::LookAndFeel* getLookAndFeel()
    {
        if (containsLookAndFeelType<LookAndFeelSubclass>())
            return lnfs[getLNFType<LookAndFeelSubclass>()].get();

        return addLookAndFeel<LookAndFeelSubclass>();
    }

    /** Adds a new look and feel to the allocator */
    template <typename LookAndFeelSubclass>
    juce::LookAndFeel* addLookAndFeel()
    {
        auto lnfType = getLNFType<LookAndFeelSubclass>();
        lnfs[lnfType] = std::make_unique<LookAndFeelSubclass>();

        return lnfs[lnfType].get();
    }

    /** Checks if the allocator already contains this look and feel */
    template <typename LookAndFeelSubclass>
    bool containsLookAndFeelType() const
    {
        return lnfs.find (getLNFType<LookAndFeelSubclass>()) != lnfs.end();
    }

private:
    template <typename LookAndFeelSubclass>
    static juce::String getLNFType()
    {
        return juce::String (typeid (LookAndFeelSubclass).name());
    }

    using LookAndFeelPtr = std::unique_ptr<juce::LookAndFeel>;
    std::unordered_map<juce::String, LookAndFeelPtr> lnfs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LNFAllocator)
};

} // namespace chowdsp
