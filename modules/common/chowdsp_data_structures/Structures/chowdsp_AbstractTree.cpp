namespace chowdsp
{
template <typename ElementType, typename DerivedType>
AbstractTree<ElementType, DerivedType>::AbstractTree()
{
    clear();
}

template <typename ElementType, typename DerivedType>
AbstractTree<ElementType, DerivedType>::~AbstractTree()
{
    doForAllNodes ([] (Node& node)
                   { node.~Node(); });
}

template <typename ElementType, typename DerivedType>
ElementType& AbstractTree<ElementType, DerivedType>::insertElement (ElementType&& elementToInsert)
{
    count++;
    return DerivedType::insertElementInternal (static_cast<DerivedType&> (*this),
                                               std::move (elementToInsert),
                                               root_node);
}

template <typename ElementType, typename DerivedType>
void AbstractTree<ElementType, DerivedType>::insertElements (std::vector<ElementType>&& elements)
{
    count += static_cast<int> (elements.size());
    for (auto& element : std::move (elements))
        DerivedType::insertElementInternal (static_cast<DerivedType&> (*this),
                                            std::move (element),
                                            root_node);
}

template <typename ElementType, typename DerivedType>
template <typename Comparator>
void AbstractTree<ElementType, DerivedType>::insertNodeSorted (Node& parent, Node* new_node, Comparator&& comparator)
{
    new_node->parent = &parent;

    // parent node has no children, so add the first one here...
    if (parent.first_child == nullptr)
    {
        parent.first_child = new_node;
        parent.last_child = new_node;
        return;
    }

    // insert into the parents children, sorted
    for (auto* iter = parent.first_child; iter != nullptr; iter = iter->next_sibling)
    {
        if (comparator (*new_node, *iter))
        {
            new_node->next_sibling = iter;
            new_node->prev_sibling = iter->prev_sibling;
            iter->prev_sibling = new_node;

            if (auto* prev_sibling = new_node->prev_sibling; prev_sibling != nullptr)
                prev_sibling->next_sibling = new_node;

            if (iter == parent.first_child)
                parent.first_child = new_node;

            return;
        }
    }

    // insert at the end of the parents children
    parent.last_child->next_sibling = new_node;
    new_node->prev_sibling = parent.last_child;
    parent.last_child = new_node;
}

template <typename ElementType, typename DerivedType>
void AbstractTree<ElementType, DerivedType>::removeNode (Node& node)
{
    if (node.parent == nullptr)
        return; // this is the root node! please don't delete me :(

    onDelete (node);

    if (node.leaf.has_value())
        count--;

    if (node.prev_sibling != nullptr)
        node.prev_sibling->next_sibling = node.next_sibling;
    if (node.next_sibling != nullptr)
        node.next_sibling->prev_sibling = node.prev_sibling;
    if (node.next_linear != nullptr)
        node.next_linear->prev_linear = node.prev_linear;
    // we don't need to check if node.prev_linear is nullptr, because the root node is never deleted!
    node.prev_linear->next_linear = node.next_linear;

    if (last_node == &node)
        last_node = node.prev_linear;

    if (node.parent->first_child == node.parent->last_child)
    {
        node.parent->first_child = nullptr;
        node.parent->last_child = nullptr;
        removeNode (*node.parent);
    }
    else
    {
        if (node.parent->first_child == &node)
            node.parent->first_child = node.next_sibling;
        if (node.parent->last_child == &node)
            node.parent->last_child = node.prev_sibling;
    }

    node.~Node();
}

template <typename ElementType, typename DerivedType>
void AbstractTree<ElementType, DerivedType>::removeElement (const ElementType& element)
{
    for (auto* node = &root_node; node != nullptr; node = node->next_linear)
    {
        if (node->leaf.has_value() && node->leaf == element)
        {
            removeNode (*node);
            break;
        }
    }
}

template <typename ElementType, typename DerivedType>
template <typename Callable>
void AbstractTree<ElementType, DerivedType>::removeElements (const Callable& elementsToRemove)
{
    // This algorithm assumes that a child node has always been allocated _after_
    // its parent. If later on we make it possible to "move" nodes, then this may
    // need to change.
    for (auto* node = &root_node; node != nullptr;)
    {
        if (node->leaf.has_value() && elementsToRemove (*node->leaf))
        {
            auto* next_node = node->next_linear;
            removeNode (*node);
            node = next_node;
        }
        else
        {
            node = node->next_linear;
        }
    }
}

template <typename ElementType, typename DerivedType>
void AbstractTree<ElementType, DerivedType>::clear()
{
    doForAllNodes ([] (Node& node)
                   { node.~Node(); });
    allocator.reset (64 * sizeof (Node));
    count = 0;
}

template <typename ElementType, typename DerivedType>
ElementType* AbstractTree<ElementType, DerivedType>::findElement (const ElementType& element)
{
    ElementType* result = nullptr;
    doForAllElements (
        [&result, element] (ElementType& candidate)
        {
            if (element == candidate)
                result = &candidate;
        });
    return result;
}

template <typename ElementType, typename DerivedType>
const ElementType* AbstractTree<ElementType, DerivedType>::findElement (const ElementType& element) const
{
    return const_cast<AbstractTree&> (*this).findElement (element); // NOSONAR
}

template <typename ElementType, typename DerivedType>
template <typename Callable>
void AbstractTree<ElementType, DerivedType>::doForAllNodes (Callable&& callable)
{
    for (auto* iter = &root_node; iter != nullptr; iter = iter->next_linear)
        callable (*iter);
}

template <typename ElementType, typename DerivedType>
template <typename Callable>
void AbstractTree<ElementType, DerivedType>::doForAllNodes (Callable&& callable) const
{
    for (auto* iter = &root_node; iter != nullptr; iter = iter->next_linear)
        callable (*iter);
}

template <typename ElementType, typename DerivedType>
template <typename Callable>
void AbstractTree<ElementType, DerivedType>::doForAllElements (Callable&& callable)
{
    doForAllNodes (
        [c = std::forward<Callable> (callable)] (Node& node)
        {
            if (node.leaf.has_value())
                c (*node.leaf);
        });
}

template <typename ElementType, typename DerivedType>
template <typename Callable>
void AbstractTree<ElementType, DerivedType>::doForAllElements (Callable&& callable) const
{
    doForAllNodes (
        [c = std::forward<Callable> (callable)] (const Node& node)
        {
            if (node.leaf.has_value())
                c (*node.leaf);
        });
}

template <typename ElementType, typename DerivedType>
typename AbstractTree<ElementType, DerivedType>::Node* AbstractTree<ElementType, DerivedType>::createEmptyNode()
{
    auto* new_node = new (allocator.allocate<Node> (1)) Node {};

    last_node->next_linear = new_node;
    new_node->prev_linear = last_node;
    last_node = new_node;

    return new_node;
}

template <typename ElementType, typename DerivedType>
std::string_view AbstractTree<ElementType, DerivedType>::allocateTag (std::string_view str)
{
    auto* str_data = allocator.allocate<char> (str.size());
    std::copy (str.begin(), str.end(), str_data);
    return { str_data, str.size() }; // NOLINT NOSONAR
}
} // namespace chowdsp
