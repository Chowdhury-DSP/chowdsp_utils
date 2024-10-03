#pragma once

#include <optional>

namespace chowdsp
{
/** An abstracted tree-like data structure (not a Binary Tree) */
template <typename ElementType, typename DerivedType>
class AbstractTree
{
public:
    struct ValuePtr
    {
        JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4324) // structure was padded due to alignment specifier
        struct alignas (8) Void
        {
        };
        PackedPointer<Void> ptr { nullptr, Empty };
        JUCE_END_IGNORE_WARNINGS_MSVC

        ValuePtr() = default;

        enum : uint8_t
        {
            Empty = 0,
            Leaf = 1,
            Tag = 2,
        };

        void set (ElementType* new_leaf)
        {
            jassert (! has_value());
            ptr.set (reinterpret_cast<Void*> (new_leaf), Leaf); // NOSONAR
        }

        void set_tag (std::string_view* new_tag)
        {
            jassert (! has_value());
            ptr.set (reinterpret_cast<Void*> (new_tag), Tag); // NOSONAR
        }

        [[nodiscard]] ElementType& leaf()
        {
            jassert (has_value());
            return *(reinterpret_cast<ElementType*> (ptr.get_ptr())); // NOSONAR
        }

        [[nodiscard]] const ElementType& leaf() const
        {
            jassert (has_value());
            return *(reinterpret_cast<const ElementType*> (ptr.get_ptr())); // NOSONAR
        }

        [[nodiscard]] std::string_view tag() const
        {
            jassert (is_tag());
            return *(reinterpret_cast<const std::string_view*> (ptr.get_ptr())); // NOSONAR
        }

        void destroy()
        {
            if (has_value())
                leaf().~ElementType(); // NOSONAR
            ptr.set (nullptr, Empty);
        }

        [[nodiscard]] bool has_value() const noexcept
        {
            return ptr.get_flags() == Leaf;
        }

        [[nodiscard]] bool is_tag() const noexcept
        {
            return ptr.get_flags() == Tag;
        }
    };

    struct Node
    {
        ValuePtr value {};

        Node* parent {}; // slot for parent in hierarchy
        Node* first_child {}; // slot for first child in hierarchy
        Node* next_sibling {}; // slot for next sibling in hierarchy
        Node* prev_sibling {}; // slot for previous sibling in hierarchy
        Node* next_linear {}; // slot for linked list through all nodes
    };

    explicit AbstractTree (size_t num_nodes_reserved = 64);
    virtual ~AbstractTree();

    AbstractTree (const AbstractTree&) = delete;
    AbstractTree& operator= (const AbstractTree&) = delete;
    AbstractTree (AbstractTree&&) noexcept = default;
    AbstractTree& operator= (AbstractTree&&) noexcept = default;

    /** Inserts an element into the tree. */
    ElementType& insertElement (ElementType&& elementToInsert);

    /** Inserts a bunch of elements into the tree. */
    void insertElements (std::vector<ElementType>&& elements);

    template <typename Comparator>
    static void insertNodeSorted (Node& parent, Node* new_node, Comparator&& comparator);

    /** Removes a node. */
    void removeNode (Node& node);

    /**
     * Removes a element by value.
     * If the tree contains multiple identical elements, only the first will be removed.
     */
    void removeElement (const ElementType& element);

    /**
     * Removes elements that meet the given criterion.
     * Callable should have the form `bool (const ElementType&)`.
     */
    template <typename Callable>
    void removeElements (const Callable& elementsToRemove);

    /** Removes all elements from the tree. */
    void clear();

    /** Returns the total number of elements contained in this tree. */
    [[nodiscard]] int size() const { return count; }

    /** Checks if the tree currently contains an element. If true, then return the element, else return nullptr. */
    [[nodiscard]] OptionalRef<ElementType> findElement (const ElementType& element);

    /** Checks if the tree currently contains an element. If true, then return the element, else return nullptr. */
    [[nodiscard]] OptionalRef<const ElementType> findElement (const ElementType& element) const;

    template <typename Callable>
    void doForAllNodes (Callable&& callable);

    template <typename Callable>
    void doForAllNodes (Callable&& callable) const;

    template <typename Callable>
    void doForAllElements (Callable&& callable);

    template <typename Callable>
    void doForAllElements (Callable&& callable) const;

    /** Creates a new tag node in the tree's memory arena. */
    Node* createTagNode (std::string_view str);

    /** Creates a new leaf node in the tree's memory arena. */
    template <typename C = ElementType, typename... Args>
    Node* createLeafNode (Args&&... args)
    {
        auto* bytes = (std::byte*) allocator.allocate_bytes (sizeof (Node) + sizeof (C) + alignof (C), alignof (Node));

        auto* new_node = new (bytes) Node {};
        last_node->next_linear = new_node;
        last_node = new_node;

        auto* new_obj = new (juce::snapPointerToAlignment (bytes + sizeof (Node), alignof (C))) C (std::forward<Args> (args)...);
        new_node->value.set (new_obj);
        return new_node;
    }

    [[nodiscard]] Node& getRootNode() noexcept { return root_node; }
    [[nodiscard]] const Node& getRootNode() const noexcept { return root_node; }

    /**
     * Reserves memory for some number of nodes.
     * NOTE: this must be called while the tree is empty.
     */
    void reserve (size_t num_nodes);

    /**
     * Shrinks the tree's memory arena down to the reserved size.
     * NOTE: this must be called while the tree is empty.
     */
    void shrinkArena();

protected:
    virtual void onDelete (const Node& /*nodeBeingDeleted*/) {}

    ChainedArenaAllocator allocator {};

private:
    Node root_node {};
    int count = 0;

    Node* last_node = &root_node;
};
} // namespace chowdsp

#include "chowdsp_AbstractTree.cpp"
