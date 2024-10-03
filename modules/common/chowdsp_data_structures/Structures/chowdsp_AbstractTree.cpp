namespace chowdsp
{
template <typename ElementType, typename DerivedType>
AbstractTree<ElementType, DerivedType>::AbstractTree (size_t num_nodes_reserved)
{
    reserve (num_nodes_reserved);
    clear();
}

template <typename ElementType, typename DerivedType>
AbstractTree<ElementType, DerivedType>::~AbstractTree()
{
    doForAllNodes ([] (Node& node)
                   { node.value.destroy(); });
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
        return;
    }

    // insert into the parents children, sorted
    Node* prev {};
    for (Node* iter = parent.first_child; iter != nullptr; iter = iter->next_sibling)
    {
        if (comparator (*new_node, *iter))
        {
            new_node->next_sibling = iter;
            new_node->prev_sibling = iter->prev_sibling;
            iter->prev_sibling = new_node;

            if (prev != nullptr)
                prev->next_sibling = new_node;

            if (iter == parent.first_child)
                parent.first_child = new_node;

            return;
        }
        prev = iter;
    }

    // insert at the end of the parents children
    prev->next_sibling = new_node;
    new_node->prev_sibling = prev;
}

template <typename ElementType, typename DerivedType>
void AbstractTree<ElementType, DerivedType>::removeNode (Node& node)
{
    if (node.parent == nullptr)
        return; // this is the root node! please don't delete me :(

    onDelete (node);

    if (node.value.has_value())
        count--;

    for (auto* iter = &root_node; iter != nullptr; iter = iter->next_linear)
    {
        if (iter->next_linear == &node)
        {
            iter->next_linear = node.next_linear;
            if (last_node == &node)
                last_node = iter;
            break;
        }
    }

    if (node.prev_sibling != nullptr)
        node.prev_sibling->next_sibling = node.next_sibling;
    if (node.next_sibling != nullptr)
        node.next_sibling->prev_sibling = node.prev_sibling;

    if (node.prev_sibling == nullptr && node.next_sibling == nullptr)
    {
        node.parent->first_child = nullptr;
        removeNode (*node.parent);
    }
    else
    {
        if (node.parent->first_child == &node)
            node.parent->first_child = node.next_sibling;
    }

    node.value.destroy();
}

template <typename ElementType, typename DerivedType>
void AbstractTree<ElementType, DerivedType>::removeElement (const ElementType& element)
{
    for (auto* node = &root_node; node != nullptr; node = node->next_linear)
    {
        if (node->value.has_value() && node->value.leaf() == element)
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
        auto* next_node = node->next_linear;
        if (node->value.has_value() && elementsToRemove (node->value.leaf()))
            removeNode (*node);

        node = next_node;
    }
}

template <typename ElementType, typename DerivedType>
void AbstractTree<ElementType, DerivedType>::clear()
{
    doForAllNodes ([] (Node& node)
                   { node.value.destroy(); });
    allocator.clear();
    count = 0;
    root_node = {};
}

template <typename ElementType, typename DerivedType>
OptionalRef<ElementType> AbstractTree<ElementType, DerivedType>::findElement (const ElementType& element)
{
    OptionalRef<ElementType> result {};
    doForAllElements (
        [&result, element] (ElementType& candidate)
        {
            if (element == candidate)
                result = candidate;
        });
    return result;
}

template <typename ElementType, typename DerivedType>
OptionalRef<const ElementType> AbstractTree<ElementType, DerivedType>::findElement (const ElementType& element) const
{
    OptionalRef<const ElementType> result {};
    doForAllElements (
        [&result, element] (const ElementType& candidate)
        {
            if (element == candidate)
                result = candidate;
        });
    return result;
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
            if (node.value.has_value())
                c (node.value.leaf());
        });
}

template <typename ElementType, typename DerivedType>
template <typename Callable>
void AbstractTree<ElementType, DerivedType>::doForAllElements (Callable&& callable) const
{
    doForAllNodes (
        [c = std::forward<Callable> (callable)] (const Node& node)
        {
            if (node.value.has_value())
                c (node.value.leaf());
        });
}

template <typename ElementType, typename DerivedType>
typename AbstractTree<ElementType, DerivedType>::Node* AbstractTree<ElementType, DerivedType>::createTagNode (std::string_view str)
{
    auto* bytes = (std::byte*) allocator.allocate_bytes (sizeof (Node) + sizeof (std::string_view) + alignof (std::string_view) + str.size(), alignof (Node));

    auto* new_node = new (bytes) Node {};
    last_node->next_linear = new_node;
    last_node = new_node;

    bytes = juce::snapPointerToAlignment (bytes + sizeof (Node), alignof (std::string_view));

    auto* str_data = (char*) (bytes + sizeof (std::string_view));
    std::copy (str.begin(), str.end(), str_data);

    auto tag_str_view = new (bytes) std::string_view { str_data, str.size() };
    new_node->value.set_tag (tag_str_view);

    return new_node;
}

template <typename ElementType, typename DerivedType>
void AbstractTree<ElementType, DerivedType>::reserve (size_t num_nodes)
{
    if (count > 0)
    {
        jassertfalse;
        return;
    }
    allocator.reset (num_nodes * (sizeof (Node) + sizeof (ElementType) + alignof (ElementType)));
}

template <typename ElementType, typename DerivedType>
void AbstractTree<ElementType, DerivedType>::shrinkArena()
{
    if (count > 0)
    {
        jassertfalse;
        return;
    }
    allocator.reset (allocator.get_current_arena().get_total_num_bytes());
}
} // namespace chowdsp
