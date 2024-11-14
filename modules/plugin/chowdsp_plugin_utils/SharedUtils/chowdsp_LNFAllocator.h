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
    LookAndFeelSubclass* getLookAndFeel()
    {
        return addLookAndFeel<LookAndFeelSubclass>();
    }

    /** Adds a new look and feel to the allocator */
    template <typename LookAndFeelSubclass>
    LookAndFeelSubclass* addLookAndFeel()
    {
        auto& lnfCacheEntry = lnfs[getLNFType<LookAndFeelSubclass>()];
        if (lnfCacheEntry != nullptr)
            return reinterpret_cast<LookAndFeelSubclass*> (lnfCacheEntry.get()); // NOSONAR

        lnfCacheEntry = std::make_unique<LookAndFeelSubclass>();
        return dynamic_cast<LookAndFeelSubclass*> (lnfCacheEntry.get());
    }

    /** Checks if the allocator already contains this look and feel */
    template <typename LookAndFeelSubclass>
    [[nodiscard]] bool containsLookAndFeelType() const
    {
        return lnfs.count (getLNFType<LookAndFeelSubclass>()) > 0;
    }

private:
    template <typename LookAndFeelSubclass>
    static std::type_index getLNFType()
    {
        return typeid (LookAndFeelSubclass);
    }

    using LookAndFeelPtr = std::unique_ptr<juce::LookAndFeel>;
    std::unordered_map<std::type_index, LookAndFeelPtr> lnfs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LNFAllocator)
};

/** Helpful alias for creating a SharedResourcePointer<LNFAllocator> */
using SharedLNFAllocator = juce::SharedResourcePointer<LNFAllocator>;

} // namespace chowdsp
