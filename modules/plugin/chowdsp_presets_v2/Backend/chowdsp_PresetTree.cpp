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

    Preset& flatInserter (Preset&& preset, std::vector<PresetTree::Item>& topLevelItems, const PresetTree::InsertionHelper& insertionHelper)
    {
        PresetTree::Item item;
        item.preset = std::move (preset);

        auto& insertedItem = insertionHelper.insertItemIntoTree (topLevelItems, std::move (item));
        jassert (insertedItem.preset.has_value());
        return *insertedItem.preset;
    }

    template <typename TagGetter, typename FallbackInserter = decltype (&flatInserter)>
    Preset& tagBasedInserter (Preset&& preset,
                              std::vector<PresetTree::Item>& topLevelItems,
                              const PresetTree::InsertionHelper& insertionHelper,
                              TagGetter&& tagGetter,
                              FallbackInserter&& fallbackInserter = &flatInserter)
    {
        const auto tag = tagGetter (preset);
        if (tag.isEmpty())
            // no category, so just add this to the top-level list
            return fallbackInserter (std::move (preset), topLevelItems, insertionHelper);

        for (auto& items : topLevelItems)
        {
            if (items.tag == tag)
                return fallbackInserter (std::move (preset), items.subtree, insertionHelper);
        }

        // preset vendor is not currently in the tree, so let's add a new sub-tree
        PresetTree::Item tree;
        tree.tag = tag;
        auto& insertedPreset = fallbackInserter (std::move (preset), tree.subtree, insertionHelper);
        insertionHelper.insertItemIntoTree (topLevelItems, std::move (tree));
        return insertedPreset;
    }

    Preset& vendorInserter (Preset&& preset, std::vector<PresetTree::Item>& topLevelItems, const PresetTree::InsertionHelper& insertionHelper)
    {
        return tagBasedInserter (std::move (preset),
                                 topLevelItems,
                                 insertionHelper,
                                 [] (const Preset& p)
                                 { return p.getVendor(); });
    }

    Preset& categoryInserter (Preset&& preset, std::vector<PresetTree::Item>& topLevelItems, const PresetTree::InsertionHelper& insertionHelper)
    {
        return tagBasedInserter (std::move (preset),
                                 topLevelItems,
                                 insertionHelper,
                                 [] (const Preset& p)
                                 { return p.getCategory(); });
    }

    Preset& vendorCategoryInserter (Preset&& preset, std::vector<PresetTree::Item>& topLevelItems, const PresetTree::InsertionHelper& insertionHelper)
    {
        return tagBasedInserter (
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
static void removePresetsGeneric (Callable&& shouldDeletePresetItem, std::vector<PresetTree::Item>& items, PresetState* presetState)
{
    VectorHelpers::erase_if (
        items,
        [&presetState, shouldDelete = std::forward<Callable> (shouldDeletePresetItem)] (const PresetTree::Item& item)
        {
            if (shouldDelete (item))
            {
                if (presetState != nullptr && presetState->get() != nullptr && *presetState->get() == *item.preset)
                    presetState->assumeOwnership();

                return true;
            }

            return false;
        });

    for (auto& item : items)
    {
        if (! item.preset.has_value())
            removePresetsGeneric (std::forward<Callable> (shouldDeletePresetItem), item.subtree, presetState);
    }

    // Remove empty sub-trees
    VectorHelpers::erase_if (items, [] (const PresetTree::Item& item)
                             { return ! item.preset.has_value() && item.subtree.empty(); });
}

PresetTree::PresetTree (PresetState* currentPresetState, InsertionHelper&& insertionHelper)
    : treeInserter (&PresetTreeInserters::flatInserter), // NOSONAR
      presetState (currentPresetState),
      insertHelper (std::move (insertionHelper))
{
    if (insertHelper.tagSortMethod == nullptr)
        insertHelper.tagSortMethod = &PresetTreeInserters::defaultTagComparator;

    if (insertHelper.presetSortMethod == nullptr)
        insertHelper.presetSortMethod = &PresetTreeInserters::defaultPresetComparator;

    if (insertHelper.insertItemIntoTree == nullptr)
        insertHelper.insertItemIntoTree = [this] (std::vector<Item>& vec, Item&& item) -> Item&
        {
            return *VectorHelpers::insert_sorted (vec,
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

const Preset& PresetTree::insertPreset (Preset&& presetToInsert)
{
    const auto& insertedPreset = treeInserter (std::move (presetToInsert), items, insertHelper);
    refreshPresetIndexes();
    return insertedPreset;
}

void PresetTree::insertPresets (std::vector<Preset>&& presets)
{
    for (auto& preset : std::move (presets))
        treeInserter (std::move (preset), items, insertHelper);
    refreshPresetIndexes();
}

void PresetTree::removePreset (int index)
{
    removePresetsGeneric ([index] (const Item& item)
                          { return item.preset.has_value() && item.presetIndex == index; },
                          items,
                          presetState);
    refreshPresetIndexes();
}

void PresetTree::removePreset (const Preset& preset)
{
    removePresetsGeneric ([&preset] (const Item& item)
                          { return item.preset.has_value() && item.preset == preset; },
                          items,
                          presetState);
    refreshPresetIndexes();
}

void PresetTree::removePresets (std::function<bool (const Preset& preset)>&& presetsToRemove)
{
    removePresetsGeneric ([&presetsToRemove] (const Item& item)
                          { return item.preset.has_value() && presetsToRemove (*item.preset); },
                          items,
                          presetState);
    refreshPresetIndexes();
}

const Preset* PresetTree::getPresetByIndex (int index) const
{
    const Preset* result = nullptr;
    if (! juce::isPositiveAndBelow (index, totalNumPresets))
        return result;

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

const Preset* PresetTree::findPreset (const Preset& preset) const
{
    const Preset* result = nullptr;
    doForAllPresetItems ([&result, &preset] (const Item& item)
                         {
                             if (item.preset.has_value() && preset == item.preset)
                                 result = &(*item.preset); },
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
