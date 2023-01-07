#include "chowdsp_PresetTree.h"

namespace chowdsp
{
namespace PresetTreeInserters
{
    bool defaultTagComparator (const juce::String& first, const juce::String& second)
    {
        return first.compare (second) < 0;
    }

    bool defaultPresetComparator (const Preset& first, const Preset& second)
    {
        return first.getName().compare (second.getName()) < 0;
    }

    void flatInserter (Preset&& preset, std::vector<PresetTree::Item>& topLevelItems, const PresetTree::InsertionHelper& insertionHelper)
    {
        PresetTree::Item item;
        item.preset = preset;
        insertionHelper.insertItemIntoTree (topLevelItems, std::move (item));
    }

    template <typename TagGetter, typename FallbackInserter = decltype (&flatInserter)>
    void tagBasedInserter (Preset&& preset,
                           std::vector<PresetTree::Item>& topLevelItems,
                           const PresetTree::InsertionHelper& insertionHelper,
                           TagGetter&& tagGetter,
                           FallbackInserter&& fallbackInserter = &flatInserter)
    {
        const auto tag = tagGetter (preset);
        if (tag.isEmpty())
        {
            // no category, so just add this to the top-level list
            fallbackInserter (std::move (preset), topLevelItems, insertionHelper);
            return;
        }

        for (auto& items : topLevelItems)
        {
            if (items.tag == tag)
            {
                fallbackInserter (std::move (preset), items.subtree, insertionHelper);
                return;
            }
        }

        // preset vendor is not currently in the tree, so let's add a new sub-tree
        PresetTree::Item tree;
        tree.tag = tag;
        fallbackInserter (std::move (preset), tree.subtree, insertionHelper);
        insertionHelper.insertItemIntoTree (topLevelItems, std::move (tree));
    }

    void vendorInserter (Preset&& preset, std::vector<PresetTree::Item>& topLevelItems, const PresetTree::InsertionHelper& insertionHelper)
    {
        tagBasedInserter (std::move (preset),
                          topLevelItems,
                          insertionHelper,
                          [] (const Preset& p)
                          { return p.getVendor(); });
    }

    void categoryInserter (Preset&& preset, std::vector<PresetTree::Item>& topLevelItems, const PresetTree::InsertionHelper& insertionHelper)
    {
        tagBasedInserter (std::move (preset),
                          topLevelItems,
                          insertionHelper,
                          [] (const Preset& p)
                          { return p.getCategory(); });
    }

    void vendorCategoryInserter (Preset&& preset, std::vector<PresetTree::Item>& topLevelItems, const PresetTree::InsertionHelper& insertionHelper)
    {
        tagBasedInserter (
            std::move (preset),
            topLevelItems,
            insertionHelper,
            [] (const Preset& p)
            { return p.getVendor(); },
            &categoryInserter);
    }
} // namespace PresetTreeInserters

template <typename Callable>
static void doForAllPresetItems (Callable&& callable, std::vector<PresetTree::Item>& items)
{
    for (auto& item : items)
    {
        if (item.preset.has_value())
            callable (item);
        else
            doForAllPresetItems (std::forward<Callable> (callable), item.subtree);
    }
}

template <typename Callable>
static void doForAllPresetItems (Callable&& callable, const std::vector<PresetTree::Item>& items)
{
    for (auto& item : items)
    {
        if (item.preset.has_value())
            callable (item);
        else
            doForAllPresetItems (std::forward<Callable> (callable), item.subtree);
    }
}

template <typename Callable>
static void removePresetsGeneric (Callable&& shouldDeletePresetItem, std::vector<PresetTree::Item>& items)
{
    VectorHelpers::erase_if (items, shouldDeletePresetItem);
    for (auto& item : items)
    {
        if (! item.preset.has_value())
            removePresetsGeneric (std::forward<Callable> (shouldDeletePresetItem), item.subtree);
    }
    VectorHelpers::erase_if (items, [] (const PresetTree::Item& item)
                             { return ! item.preset.has_value() && item.subtree.empty(); });
}

PresetTree::PresetTree (InsertionHelper&& insertionHelper)
    : treeInserter (&PresetTreeInserters::flatInserter),
      insertHelper (std::move (insertionHelper))
{
    if (insertHelper.tagSortMethod == nullptr)
        insertHelper.tagSortMethod = &PresetTreeInserters::defaultTagComparator;

    if (insertHelper.presetSortMethod == nullptr)
        insertHelper.presetSortMethod = &PresetTreeInserters::defaultPresetComparator;

    if (insertHelper.insertItemIntoTree == nullptr)
        insertHelper.insertItemIntoTree = [this] (std::vector<Item>& vec, Item&& item)
        {
            VectorHelpers::insert_sorted (vec,
                                          std::move (item),
                                          [&tagComp = std::as_const (insertHelper.tagSortMethod),
                                           &presetComp = std::as_const (insertHelper.presetSortMethod)] (const Item& item1, const Item& item2)
                                          {
                                              if (item1.preset.has_value() && ! item2.preset.has_value())
                                                  return false;

                                              if (! item1.preset.has_value() && item2.preset.has_value())
                                                  return true;

                                              if (item1.preset.has_value())
                                                  return presetComp (*item1.preset, *item2.preset);

                                              return tagComp (item1.tag, item2.tag);
                                          });
        };
}

PresetTree::~PresetTree() = default;

void PresetTree::insertPreset (Preset&& presetToInsert)
{
    treeInserter (std::move (presetToInsert), items, insertHelper);
    refreshPresetIndexes();
}

void PresetTree::insertPresets (std::vector<Preset>&& presets)
{
    for (auto& preset : presets)
        treeInserter (std::move (preset), items, insertHelper);
    refreshPresetIndexes();
}

void PresetTree::removePreset (int index)
{
    removePresetsGeneric ([index] (const Item& item)
                          { return item.preset.has_value() && item.presetIndex == index; },
                          items);
    refreshPresetIndexes();
}

void PresetTree::removePreset (const Preset& preset)
{
    removePresetsGeneric ([&preset] (const Item& item)
                          { return item.preset.has_value() && item.preset == preset; },
                          items);
    refreshPresetIndexes();
}

void PresetTree::removePresets (std::function<bool (const Preset& preset)>&& presetsToRemove)
{
    removePresetsGeneric ([&presetsToRemove] (const Item& item)
                          { return item.preset.has_value() && presetsToRemove (*item.preset); },
                          items);
    refreshPresetIndexes();
}

const Preset* PresetTree::getPresetByIndex (int index) const
{
    const Preset* result = nullptr;
    if (! juce::isPositiveAndBelow (index, totalNumPresets))
    {
        jassertfalse;
        return result;
    }

    doForAllPresetItems ([&result, index] (const Item& item)
                         {
                             if (item.presetIndex == index)
                                 result = &(*item.preset); },
                         items);
    return result;
}

int PresetTree::getIndexForPreset (const Preset& preset) const
{
    int result = -1;
    doForAllPresetItems ([&result, &preset] (const Item& item)
                         {
                             if (item.preset.has_value() && preset == item.preset)
                                 result = item.presetIndex; },
                         items);
    return result;
}

void PresetTree::refreshPresetIndexes()
{
    int index = 0;
    doForAllPresetItems ([&index] (Item& item) mutable
                         { item.presetIndex = index++; },
                         items);
    totalNumPresets = index;
}
} // namespace chowdsp
