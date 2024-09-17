#include "chowdsp_PresetTree.h"

namespace chowdsp::presets
{
namespace PresetTreeInserters
{
    bool defaultTagComparator (std::string_view first, std::string_view second)
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
        auto* item = tree.createLeafNode (std::move (preset));
        insertionHelper.insertNodeIntoTree (root, item);
        return item->value.leaf();
    }

    template <typename TagGetter, typename FallbackInserter = decltype (&flatInserter)>
    Preset& tagBasedInserter (Preset&& preset,
                              PresetTree& tree,
                              PresetTree::Node& root,
                              const PresetTree::InsertionHelper& insertionHelper,
                              TagGetter&& tagGetter,
                              FallbackInserter&& fallbackInserter = &flatInserter)
    {
        const juce::String& tagString = tagGetter (preset);
        const auto tag = std::string_view { tagString.toRawUTF8(), tagString.getNumBytesAsUTF8() };
        if (tag.empty())
        {
            // no category, so just add this to the top-level list
            return fallbackInserter (std::move (preset), tree, root, insertionHelper);
        }

        for (auto* iter = root.first_child; iter != nullptr; iter = iter->next_sibling)
        {
            if (iter->value.tag() == tag)
                return fallbackInserter (std::move (preset), tree, *iter, insertionHelper);
        }

        // preset vendor is not currently in the tree, so let's add a new sub-tree
        auto* subTree = tree.createTagNode (tag);
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
                if (item1.value.has_value() && ! item2.value.has_value())
                    return false;

                if (! item1.value.has_value() && item2.value.has_value())
                    return true;

                if (item1.value.has_value())
                    return presetComp (item1.value.leaf(), item2.value.leaf());

                return tagComp (item1.value.tag(), item2.value.tag());
            };
            insertNodeSorted (parent, newNode, comparator);
        };
    }
}

Preset& PresetTree::insertElementInternal (PresetTree& self, Preset&& element, Node& root)
{
    return self.treeInserter (std::move (element), self, root, self.insertHelper);
}

void PresetTree::onDelete (const Node& nodeBeingDeleted)
{
    if (presetState != nullptr
        && presetState->get() != nullptr
        && nodeBeingDeleted.value.has_value()
        && *presetState->get() == nodeBeingDeleted.value.leaf())
        presetState->assumeOwnership();
}
} // namespace chowdsp::presets
