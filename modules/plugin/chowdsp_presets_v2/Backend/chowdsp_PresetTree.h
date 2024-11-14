#pragma once

namespace chowdsp::presets
{
/** A tree-like data structure for storing presets. */
class PresetTree : public AbstractTree<Preset, PresetTree>
{
public:
    /** Methods for sorting and inserting presets into the tree. */
    struct InsertionHelper
    {
        std::function<bool (std::string_view, std::string_view)> tagSortMethod = nullptr;
        std::function<bool (const Preset&, const Preset&)> presetSortMethod = nullptr;
        std::function<void (Node& parent, Node* newNode)> insertNodeIntoTree = nullptr;
    };

    explicit PresetTree (PresetState* presetState = nullptr, InsertionHelper&& insertionHelper = { nullptr, nullptr, nullptr });
    ~PresetTree() override = default;

    /**
     * Method for inserting presets into the tree.
     *
     * Uses PresetTreeInserters::flatInserter by default.
     */
    std::function<Preset&(Preset&&, PresetTree&, Node& root, const InsertionHelper&)> treeInserter {};

    /** For internal use only! */
    static Preset& insertElementInternal (PresetTree& self, Preset&& element, Node& root);

protected:
    void onDelete (const Node& nodeBeingDeleted) override;

private:
    PresetState* presetState = nullptr;

    InsertionHelper insertHelper;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetTree)
};

namespace PresetTreeInserters
{
    /** Default comparator for sorting tags. */
    bool defaultTagComparator (std::string_view, std::string_view);

    /** Default comparator for sorting presets. */
    bool defaultPresetComparator (const Preset&, const Preset&);

    /** Inserts all presets into the tree as a flat list. */
    Preset& flatInserter (Preset&&, PresetTree&, PresetTree::Node&, const PresetTree::InsertionHelper&);

    /** Inserts presets into the tree with sub-trees for each vendor. */
    Preset& vendorInserter (Preset&&, PresetTree&, PresetTree::Node&, const PresetTree::InsertionHelper&);

    /** Inserts presets into the tree with sub-trees for each category. */
    Preset& categoryInserter (Preset&&, PresetTree&, PresetTree::Node&, const PresetTree::InsertionHelper&);

    /** Inserts presets into the tree with sub-trees for each vendor, and sub-sub-trees for each category. */
    Preset& vendorCategoryInserter (Preset&&, PresetTree&, PresetTree::Node&, const PresetTree::InsertionHelper&);
} // namespace PresetTreeInserters
} // namespace chowdsp::presets
