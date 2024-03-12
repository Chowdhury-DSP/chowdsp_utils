#pragma once

#include <optional>

namespace chowdsp
{
/** An abstracted tree-like data structure (not a Binary Tree) */
template <typename ElementType>
class AbstractTree
{
public:
    struct Node
    {
        std::optional<ElementType> leaf { std::nullopt };

        Node* parent {}; // slot for parent in hierarchy
        Node* first_child {}; // slot for first child in hierarchy
        Node* last_child {}; // slot for last child in hierarchy
        Node* next_sibling {}; // slot for next sibling in hierarchy
        Node* prev_sibling {}; // slot for previous sibling in hierarchy
        Node* next_linear {}; // slot for linked list through all nodes
        Node* prev_linear {}; // slot for linked list through all nodes

        std::string tag {};

        struct Locator // bucket array locator
        {
            size_t bucket_index;
            size_t slot_index;
        } locator;
    };

    AbstractTree();
    virtual ~AbstractTree();

    AbstractTree (const AbstractTree&) = delete;
    AbstractTree& operator= (const AbstractTree&) = delete;
    AbstractTree (AbstractTree&&) noexcept = default;
    AbstractTree& operator= (AbstractTree&&) noexcept = default;

    /**
     * Inserts an element into the tree.
     * Calling this invalidates any existing element indices.
     *
     * Note that if you need to insert a bunch of elements,
     * the insertElements() method will be significantly faster.
     */
    ElementType& insertElement (ElementType&& elementToInsert);

    /**
     * Inserts a bunch of elements into the tree.
     * Calling this invalidates any existing element indices.
     */
    void insertElements (std::vector<ElementType>&& elements);

    template <typename Comparator>
    static void insertNodeSorted (Node& parent, Node* new_node, Comparator&& comparator);

    /** Removes a node. */
    void removeNode (Node& node);

    /** Removes a element by value. */
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
    [[nodiscard]] ElementType* findElement (const ElementType& element);

    /** Checks if the tree currently contains an element. If true, then return the element, else return nullptr. */
    [[nodiscard]] const ElementType* findElement (const ElementType& element) const;

    template <typename Callable>
    void doForAllNodes (Callable&& callable);

    template <typename Callable>
    void doForAllNodes (Callable&& callable) const;

    template <typename Callable>
    void doForAllElements (Callable&& callable);

    template <typename Callable>
    void doForAllElements (Callable&& callable) const;

    Node* createEmptyNode();

    Node& getRootNode() noexcept { return root_node; }
    const Node& getRootNode() const noexcept { return root_node; }

protected:
    virtual ElementType& insertElementInternal (ElementType&& element, Node& root_node) = 0;
    virtual void onDelete (const Node& /*nodeBeingDeleted*/) {}

private:
    BucketArray<Node, 32> nodes;

    Node root_node {};
    int count = 0;

    Node* last_node = &root_node;
};
} // namespace chowdsp

#include "chowdsp_AbstractTree.cpp"
