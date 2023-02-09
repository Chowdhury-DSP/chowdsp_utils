#pragma once

namespace chowdsp::presets
{
/** A tree-like data structure for storing presets. */
class PresetTree
{
public:
    /** Item type used by the tree internally. */
    struct Item
    {
        std::optional<Preset> preset {}; // preset field
        int presetIndex = -1; // index for the preset in the tree

        std::vector<Item> subtree {}; // nested tree field
        juce::String tag; // tag for the nested tree
    };

    /** Methods for sorting and inserting presets into the tree. */
    struct InsertionHelper
    {
        std::function<bool (const juce::String&, const juce::String&)> tagSortMethod = nullptr;
        std::function<bool (const Preset&, const Preset&)> presetSortMethod = nullptr;
        std::function<Item&(std::vector<Item>&, Item&&)> insertItemIntoTree = nullptr;
    };

    explicit PresetTree (PresetState* presetState = nullptr, InsertionHelper&& insertionHelper = { nullptr, nullptr, nullptr });
    ~PresetTree();

    /**
     * Inserts a preset into the tree.
     * Calling this invalidates any existing preset indices.
     */
    const Preset& insertPreset (Preset&& presetToInsert);

    /**
     * Inserts a bunch of presets into the tree.
     * Calling this invalidates any existing preset indices.
     */
    void insertPresets (std::vector<Preset>&& presets);

    /** Removes a preset by index. */
    void removePreset (int index);

    /** Removes a preset by value. */
    void removePreset (const Preset& preset);

    /** Removes presets that meet the given criterion. */
    void removePresets (std::function<bool (const Preset& preset)>&& presetsToRemove);

    /** Returns the total number of presets contained in this tree. */
    [[nodiscard]] int getTotalNumberOfPresets() const { return totalNumPresets; }

    /** Returns a pointer to a preset by index, or nullptr if the index is out of range. */
    [[nodiscard]] const Preset* getPresetByIndex (int index) const;

    /** Returns the index of a preset, or -1 if the preset is not present int the tree. */
    [[nodiscard]] int getIndexForPreset (const Preset& preset) const;

    /** Checks if the tree currently contains a preset. If true, then return the preset, else return nullptr. */
    [[nodiscard]] const Preset* findPreset (const Preset& preset) const;

    /**
     * Method for inserting presets into the tree.
     *
     * Uses PresetTreeInserters::flatInserter by default.
     */
    std::function<Preset&(Preset&&, std::vector<Item>&, const InsertionHelper&)> treeInserter;

    [[nodiscard]] auto& getTreeItems() { return items; }
    [[nodiscard]] const auto& getTreeItems() const { return items; }

private:
    void refreshPresetIndexes();

    std::vector<Item> items;
    int totalNumPresets = 0;

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
    Preset& flatInserter (Preset&&, std::vector<PresetTree::Item>&, const PresetTree::InsertionHelper&);

    /** Inserts presets into the tree with sub-trees for each vendor. */
    Preset& vendorInserter (Preset&&, std::vector<PresetTree::Item>&, const PresetTree::InsertionHelper&);

    /** Inserts presets into the tree with sub-trees for each category. */
    Preset& categoryInserter (Preset&&, std::vector<PresetTree::Item>&, const PresetTree::InsertionHelper&);

    /** Inserts presets into the tree with sub-trees for each vendor, and sub-sub-trees for each category. */
    Preset& vendorCategoryInserter (Preset&&, std::vector<PresetTree::Item>&, const PresetTree::InsertionHelper&);
} // namespace PresetTreeInserters
} // namespace chowdsp::presets
