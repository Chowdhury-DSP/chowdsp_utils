#include "chowdsp_PresetTree.h"

namespace chowdsp::presets
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

    Preset& flatInserter (Preset&& preset, std::vector<PresetTree::Node>& topLevelNodes, const PresetTree::InsertionHelper& insertionHelper)
    {
        PresetTree::Node item;
        item.leaf = std::move (preset);

        auto& insertedNode = insertionHelper.insertNodeIntoTree (topLevelNodes, std::move (item));
        jassert (insertedNode.leaf.has_value());
        return *insertedNode.leaf;
    }

    template <typename TagGetter, typename FallbackInserter = decltype (&flatInserter)>
    Preset& tagBasedInserter (Preset&& preset,
                              std::vector<PresetTree::Node>& topLevelNodes,
                              const PresetTree::InsertionHelper& insertionHelper,
                              TagGetter&& tagGetter,
                              FallbackInserter&& fallbackInserter = &flatInserter)
    {
        const auto tag = tagGetter (preset);
        if (tag.isEmpty())
            // no category, so just add this to the top-level list
            return fallbackInserter (std::move (preset), topLevelNodes, insertionHelper);

        for (auto& items : topLevelNodes)
        {
            if (items.tag == tag)
                return fallbackInserter (std::move (preset), items.subtree, insertionHelper);
        }

        // preset vendor is not currently in the tree, so let's add a new sub-tree
        PresetTree::Node tree;
        tree.tag = tag.toStdString();
        auto& insertedTree = insertionHelper.insertNodeIntoTree (topLevelNodes, std::move (tree));
        return fallbackInserter (std::move (preset), insertedTree.subtree, insertionHelper);
    }

    Preset& vendorInserter (Preset&& preset, std::vector<PresetTree::Node>& topLevelNodes, const PresetTree::InsertionHelper& insertionHelper)
    {
        return tagBasedInserter (std::move (preset),
                                 topLevelNodes,
                                 insertionHelper,
                                 [] (const Preset& p)
                                 { return p.getVendor(); });
    }

    Preset& categoryInserter (Preset&& preset, std::vector<PresetTree::Node>& topLevelNodes, const PresetTree::InsertionHelper& insertionHelper)
    {
        return tagBasedInserter (std::move (preset),
                                 topLevelNodes,
                                 insertionHelper,
                                 [] (const Preset& p)
                                 { return p.getCategory(); });
    }

    Preset& vendorCategoryInserter (Preset&& preset, std::vector<PresetTree::Node>& topLevelNodes, const PresetTree::InsertionHelper& insertionHelper)
    {
        return tagBasedInserter (
            std::move (preset),
            topLevelNodes,
            insertionHelper,
            [] (const Preset& p)
            { return p.getVendor(); },
            &categoryInserter);
    }
} // namespace PresetTreeInserters

PresetTree::PresetTree (PresetState* currentPresetState, InsertionHelper&& insertionHelper)
    : treeInserter (&PresetTreeInserters::flatInserter), // NOSONAR
      presetState (currentPresetState),
      insertHelper (std::move (insertionHelper))
{
    if (insertHelper.tagSortMethod == nullptr)
        insertHelper.tagSortMethod = &PresetTreeInserters::defaultTagComparator;

    if (insertHelper.presetSortMethod == nullptr)
        insertHelper.presetSortMethod = &PresetTreeInserters::defaultPresetComparator;

    if (insertHelper.insertNodeIntoTree == nullptr)
        insertHelper.insertNodeIntoTree = [this] (std::vector<Node>& vec, Node&& item) -> Node&
        {
            return *VectorHelpers::insert_sorted (vec,
                                                  std::move (item),
                                                  [&tagComp = std::as_const (insertHelper.tagSortMethod),
                                                   &presetComp = std::as_const (insertHelper.presetSortMethod)] (const Node& item1, const Node& item2)
                                                  {
                                                      if (item1.leaf.has_value() && ! item2.leaf.has_value())
                                                          return false;

                                                      if (! item1.leaf.has_value() && item2.leaf.has_value())
                                                          return true;

                                                      if (item1.leaf.has_value())
                                                          return presetComp (*item1.leaf, *item2.leaf);

                                                      return tagComp (item1.tag, item2.tag);
                                                  });
        };
}

PresetTree::~PresetTree() = default;

Preset& PresetTree::insertElementInternal (Preset&& element, std::vector<Node>& topLevelNodes)
{
    return treeInserter (std::move (element), topLevelNodes, insertHelper);
}

void PresetTree::onDelete (const Node& nodeBeingDeleted)
{
    if (presetState != nullptr && presetState->get() != nullptr && *presetState->get() == *nodeBeingDeleted.leaf)
        presetState->assumeOwnership();
}
} // namespace chowdsp::presets
