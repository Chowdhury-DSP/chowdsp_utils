#pragma once

namespace chowdsp::presets::frontend
{
NextPrevious::NextPrevious (PresetManager& manager) : presetManager (manager)
{
}

static const PresetTree::Node* findNodeForPreset (const PresetTree::Node& root, const Preset& current)
{
    for (auto* node = &root; node != nullptr; node = node->next_linear)
    {
        if (node->value.has_value() && node->value.leaf() == current)
            return node;
    }
    return nullptr;
}

static const PresetTree::Node* getNextOrPreviousChildPresetNode (const PresetTree::Node* node, bool forward)
{
    if (forward)
        return node->first_child;

    node = node->first_child;
    while (node->next_sibling != nullptr)
        node = node->next_sibling;
    return node;
}

static const PresetTree::Node* getNextOrPreviousPresetNode (const PresetTree::Node* node, bool forward)
{
    if (node == nullptr)
        return nullptr;

    const auto* next = forward ? node->next_sibling : node->prev_sibling;
    if (next != nullptr)
        return next;

    auto* nextParent = getNextOrPreviousPresetNode (node->parent, forward);
    if (nextParent == nullptr)
        return nullptr;
    if (nextParent->value.has_value())
        return nextParent;

    return getNextOrPreviousChildPresetNode (nextParent, forward);
}

bool NextPrevious::navigateThroughPresets (bool forward)
{
    const auto& presetTree = presetManager.getPresetTree();
    const auto* currentPreset = presetManager.getCurrentPreset();
    if (currentPreset == nullptr)
        return false;

    auto* currentPresetNode = findNodeForPreset (presetTree.getRootNode(), *currentPreset);
    if (currentPresetNode == nullptr)
        return false;

    const auto* nextPresetNode = getNextOrPreviousPresetNode (currentPresetNode, forward);
    if (nextPresetNode == nullptr)
    {
        if (! wrapAtEndOfTree || presetTree.size() == 0)
            return false;

        nextPresetNode = &presetTree.getRootNode();
    }

    while (! nextPresetNode->value.has_value())
        nextPresetNode = getNextOrPreviousChildPresetNode (nextPresetNode, forward);

    presetManager.loadPreset (nextPresetNode->value.leaf());
    return true;
}

bool NextPrevious::goToNextPreset()
{
    return navigateThroughPresets (true);
}

bool NextPrevious::goToPreviousPreset()
{
    return navigateThroughPresets (false);
}
} // namespace chowdsp::presets::frontend
