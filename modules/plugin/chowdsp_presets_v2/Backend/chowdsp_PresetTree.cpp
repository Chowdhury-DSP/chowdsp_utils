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

    Preset& flatInserter (Preset&& preset,
                          PresetTree& tree,
                          PresetTree::Node& root,
                          const PresetTree::InsertionHelper& insertionHelper)
    {
        auto* item = tree.createEmptyNode();
        item->leaf = std::move (preset);

        insertionHelper.insertNodeIntoTree (root, item);
        return *item->leaf;
    }

    template <typename TagGetter, typename FallbackInserter = decltype (&flatInserter)>
    Preset& tagBasedInserter (Preset&& preset,
                              PresetTree& tree,
                              PresetTree::Node& root,
                              const PresetTree::InsertionHelper& insertionHelper,
                              TagGetter&& tagGetter,
                              FallbackInserter&& fallbackInserter = &flatInserter)
    {
        const auto tag = tagGetter (preset);
        if (tag.isEmpty())
        {
            // no category, so just add this to the top-level list
            return fallbackInserter (std::move (preset), tree, root, insertionHelper);
        }

        for (auto* iter = root.first_child; iter != nullptr; iter = iter->next_sibling)
        {
            if (iter->tag == tag)
                return fallbackInserter (std::move (preset), tree, *iter, insertionHelper);
        }

        // preset vendor is not currently in the tree, so let's add a new sub-tree
        auto* subTree = tree.createEmptyNode();
        subTree->tag = tag.toStdString();
        insertionHelper.insertNodeIntoTree (root, subTree);
        return fallbackInserter (std::move (preset), tree, *subTree, insertionHelper);
    }

    Preset& vendorInserter (Preset&& preset,
                            PresetTree& tree,
                            PresetTree::Node& root,
                            const PresetTree::InsertionHelper& insertionHelper)
    {
        return tagBasedInserter (std::move (preset),
                                 tree,
                                 root,
                                 insertionHelper,
                                 [] (const Preset& p)
                                 { return p.getVendor(); });
    }

    Preset& categoryInserter (Preset&& preset,
                              PresetTree& tree,
                              PresetTree::Node& root,
                              const PresetTree::InsertionHelper& insertionHelper)
    {
        return tagBasedInserter (std::move (preset),
                                 tree,
                                 root,
                                 insertionHelper,
                                 [] (const Preset& p)
                                 { return p.getCategory(); });
    }

    Preset& vendorCategoryInserter (Preset&& preset,
                                    PresetTree& tree,
                                    PresetTree::Node& root,
                                    const PresetTree::InsertionHelper& insertionHelper)
    {
        return tagBasedInserter (
            std::move (preset),
            tree,
            root,
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
    {
        insertHelper.insertNodeIntoTree = [this] (Node& parent, Node* newNode)
        {
            const auto comparator = [&tagComp = std::as_const (insertHelper.tagSortMethod),
                                     &presetComp = std::as_const (insertHelper.presetSortMethod)] (const Node& item1, const Node& item2)
            {
                if (item1.leaf.has_value() && ! item2.leaf.has_value())
                    return false;

                if (! item1.leaf.has_value() && item2.leaf.has_value())
                    return true;

                if (item1.leaf.has_value())
                    return presetComp (*item1.leaf, *item2.leaf);

                return tagComp (item1.tag, item2.tag);
            };
            insertNodeSorted (parent, newNode, comparator);
        };
    }
}

Preset& PresetTree::insertElementInternal (Preset&& element, Node& root)
{
    return treeInserter (std::move (element), *this, root, insertHelper);
}

void PresetTree::onDelete (const Node& nodeBeingDeleted)
{
    if (presetState != nullptr && presetState->get() != nullptr && *presetState->get() == *nodeBeingDeleted.leaf)
        presetState->assumeOwnership();
}
} // namespace chowdsp::presets
