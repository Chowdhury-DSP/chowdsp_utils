namespace chowdsp
{
#ifndef DOXYGEN
namespace abstract_tree_detail
{
    template <typename Callable, typename Node, typename Alloc>
    void doForAllNodes (Callable&& callable, std::vector<Node, Alloc>& nodes)
    {
        for (auto& node : nodes)
        {
            if (node.leaf.has_value())
                callable (node);
            else
                doForAllNodes (std::forward<Callable> (callable), node.subtree);
        }
    }

    template <typename Callable, typename Node, typename Alloc>
    void doForAllNodes (Callable&& callable, const std::vector<Node, Alloc>& nodes)
    {
        for (auto& node : nodes)
        {
            if (node.leaf.has_value())
                callable (node);
            else
                doForAllNodes (std::forward<Callable> (callable), node.subtree);
        }
    }

    template <typename DeleteChecker, typename OnDeleteAction, typename Node, typename Alloc>
    static void removeElementsGeneric (DeleteChecker&& shouldDeleteElement, OnDeleteAction&& onDeleteAction, std::vector<Node, Alloc>& nodes)
    {
        VectorHelpers::erase_if (
            nodes,
            [shouldDelete = std::forward<DeleteChecker> (shouldDeleteElement),
             onDelete = std::forward<OnDeleteAction> (onDeleteAction)] (const Node& node)
            {
                if (shouldDelete (node))
                {
                    onDelete (node);
                    return true;
                }

                return false;
            });

        for (auto& node : nodes)
        {
            if (! node.leaf.has_value())
                removeElementsGeneric (std::forward<DeleteChecker> (shouldDeleteElement),
                                       std::forward<OnDeleteAction> (onDeleteAction),
                                       node.subtree);
        }

        // Remove empty sub-trees
        VectorHelpers::erase_if (nodes, [] (const Node& node)
                                 { return ! node.leaf.has_value() && node.subtree.empty(); });
    }
} // namespace abstract_tree_detail
#endif // DOXYGEN

template <typename ElementType>
ElementType& AbstractTree<ElementType>::insertElement (ElementType&& elementToInsert)
{
    auto& insertedElement = insertElementInternal (std::move (elementToInsert), nodes);
    refreshElementIndexes();
    return insertedElement;
}

template <typename ElementType>
void AbstractTree<ElementType>::insertElements (std::vector<ElementType>&& elements)
{
    for (auto& element : std::move (elements))
        insertElementInternal (std::move (element), nodes);
    refreshElementIndexes();
}

template <typename ElementType>
void AbstractTree<ElementType>::removeElement (int index)
{
    abstract_tree_detail::removeElementsGeneric ([index] (const Node& node)
                                                 { return node.leaf.has_value() && node.leafIndex == index; },
                                                 [this] (const Node& node)
                                                 { onDelete (node); },
                                                 nodes);
    refreshElementIndexes();
}

template <typename ElementType>
void AbstractTree<ElementType>::removeElement (const ElementType& element)
{
    abstract_tree_detail::removeElementsGeneric ([&element] (const Node& node)
                                                 { return node.leaf.has_value() && node.leaf == element; },
                                                 [this] (const Node& node)
                                                 { onDelete (node); },
                                                 nodes);
    refreshElementIndexes();
}

template <typename ElementType>
void AbstractTree<ElementType>::removeElements (const std::function<bool (const ElementType&)>& elementsToRemove)
{
    abstract_tree_detail::removeElementsGeneric ([&elementsToRemove] (const Node& node)
                                                 { return node.leaf.has_value() && elementsToRemove (*node.leaf); },
                                                 [this] (const Node& node)
                                                 { onDelete (node); },
                                                 nodes);
    refreshElementIndexes();
}

template <typename ElementType>
void AbstractTree<ElementType>::clear()
{
    abstract_tree_detail::removeElementsGeneric ([] (const Node&)
                                                 { return true; },
                                                 [this] (const Node& node)
                                                 { onDelete (node); },
                                                 nodes);
    refreshElementIndexes();
}

template <typename ElementType>
ElementType* AbstractTree<ElementType>::getElementByIndex (int index)
{
    ElementType* result = nullptr;
    if (! juce::isPositiveAndBelow (index, totalNumElements))
        return result;

    abstract_tree_detail::doForAllNodes (
        [&result, index] (Node& node)
        {
            if (node.leafIndex == index)
                result = &(*node.leaf);
        },
        nodes);
    return result;
}

template <typename ElementType>
const ElementType* AbstractTree<ElementType>::getElementByIndex (int index) const
{
    return const_cast<AbstractTree&> (*this).getElementByIndex (index); // NOSONAR
}

template <typename ElementType>
int AbstractTree<ElementType>::getIndexForElement (const ElementType& element) const
{
    int result = -1;
    abstract_tree_detail::doForAllNodes (
        [&result, &element] (const Node& node)
        {
            if (node.leaf.has_value() && element == node.leaf)
                result = node.leafIndex;
        },
        nodes);
    return result;
}

template <typename ElementType>
ElementType* AbstractTree<ElementType>::findElement (const ElementType& element)
{
    ElementType* result = nullptr;
    abstract_tree_detail::doForAllNodes (
        [&result, &element] (Node& node)
        {
            if (node.leaf.has_value() && element == node.leaf)
                result = &(*node.leaf);
        },
        nodes);
    return result;
}

template <typename ElementType>
const ElementType* AbstractTree<ElementType>::findElement (const ElementType& element) const
{
    return const_cast<AbstractTree&> (*this).findElement (element); // NOSONAR
}

template <typename ElementType>
template <typename Callable>
void AbstractTree<ElementType>::doForAllNodes (Callable&& callable)
{
    abstract_tree_detail::doForAllNodes (std::forward<Callable> (callable), nodes);
}

template <typename ElementType>
template <typename Callable>
void AbstractTree<ElementType>::doForAllNodes (Callable&& callable) const
{
    abstract_tree_detail::doForAllNodes (std::forward<Callable> (callable), nodes);
}

template <typename ElementType>
template <typename Callable>
void AbstractTree<ElementType>::doForAllElements (Callable&& callable)
{
    abstract_tree_detail::doForAllNodes ([c = std::forward<Callable> (callable)] (auto& node)
                                         { c (*node.leaf); },
                                         nodes);
}

template <typename ElementType>
template <typename Callable>
void AbstractTree<ElementType>::doForAllElements (Callable&& callable) const
{
    abstract_tree_detail::doForAllNodes ([c = std::forward<Callable> (callable)] (const auto& node)
                                         { c (*node.leaf); },
                                         nodes);
}

template <typename ElementType>
void AbstractTree<ElementType>::refreshElementIndexes()
{
    int counter = 0;
    abstract_tree_detail::doForAllNodes ([&counter] (Node& node) mutable
                                         { node.leafIndex = counter++; },
                                         nodes);
    totalNumElements = counter;
}
} // namespace chowdsp
