#pragma once

namespace chowdsp::presets
{
/** A tree-like data structure for storing presets. */
class PresetTree : public AbstractTree<Preset>
{
public:
    /** Methods for sorting and inserting presets into the tree. */
    struct InsertionHelper
    {
        std::function<bool (const juce::String&, const juce::String&)> tagSortMethod = nullptr;
        std::function<bool (const Preset&, const Preset&)> presetSortMethod = nullptr;
        std::function<Node&(std::vector<Node>&, Node&&)> insertNodeIntoTree = nullptr;
    };

    explicit PresetTree (PresetState* presetState = nullptr, InsertionHelper&& insertionHelper = { nullptr, nullptr, nullptr });
    ~PresetTree();

    /**
     * Method for inserting presets into the tree.
     *
     * Uses PresetTreeInserters::flatInserter by default.
     */
    std::function<Preset&(Preset&&, std::vector<Node>&, const InsertionHelper&)> treeInserter;

protected:
    Preset& insertElementInternal (Preset&& element, std::vector<Node>& topLevelNodes) override;
    void onDelete (const Node& nodeBeingDeleted) override;

private:
    PresetState* presetState = nullptr;

    InsertionHelper insertHelper;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetTree)
};

namespace PresetTreeInserters
{
    /** Default comparator for sorting tags. */
    bool defaultTagComparator (const juce::String&, const juce::String&);

    /** Default comparator for sorting presets. */
    bool defaultPresetComparator (const Preset&, const Preset&);

    /** Inserts all presets into the tree as a flat list. */
    Preset& flatInserter (Preset&&, std::vector<PresetTree::Node>&, const PresetTree::InsertionHelper&);

    /** Inserts presets into the tree with sub-trees for each vendor. */
    Preset& vendorInserter (Preset&&, std::vector<PresetTree::Node>&, const PresetTree::InsertionHelper&);

    /** Inserts presets into the tree with sub-trees for each category. */
    Preset& categoryInserter (Preset&&, std::vector<PresetTree::Node>&, const PresetTree::InsertionHelper&);

    /** Inserts presets into the tree with sub-trees for each vendor, and sub-sub-trees for each category. */
    Preset& vendorCategoryInserter (Preset&&, std::vector<PresetTree::Node>&, const PresetTree::InsertionHelper&);
} // namespace PresetTreeInserters
} // namespace chowdsp::presets
