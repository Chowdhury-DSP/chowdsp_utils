#pragma once

#include <optional>
#include <vector>

namespace chowdsp
{
/** An abstracted tree-like data structure (not a Binary Tree) */
template <typename ElementType>
class AbstractTree
{
public:
    /** Node type used by the tree internally. */
    struct Node
    {
        std::optional<ElementType> leaf {};
        int leafIndex = -1; // Internal use only!

        using NodeAllocator = short_alloc::short_alloc<Node, 8192, 8>;
        using NodeArena = typename NodeAllocator::arena_type;
        std::vector<Node, typename Node::NodeAllocator> subtree {};
        std::string tag;

        explicit Node (NodeArena& arena) : subtree (arena) {}
    };

    using NodeVector = typename std::vector<Node, typename Node::NodeAllocator>;

    AbstractTree() = default;
    virtual ~AbstractTree() = default;

    AbstractTree (const AbstractTree&) = default;
    AbstractTree& operator= (const AbstractTree&) = default;
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

    /** Removes a element by index. */
    void removeElement (int index);

    /** Removes a element by value. */
    void removeElement (const ElementType& element);

    /** Removes elements that meet the given criterion. */
    void removeElements (const std::function<bool (const ElementType&)>& elementsToRemove);

    /** Removes all elements from the tree. */
    void clear();

    /** Returns the total number of elements contained in this tree. */
    [[nodiscard]] int size() const { return totalNumElements; }

    /** Returns a pointer to an element by index, or nullptr if the index is out of range. */
    [[nodiscard]] ElementType* getElementByIndex (int index);

    /** Returns a pointer to an element by index, or nullptr if the index is out of range. */
    [[nodiscard]] const ElementType* getElementByIndex (int index) const;

    /** Returns the index of an element, or -1 if the element is not present in the tree. */
    [[nodiscard]] int getIndexForElement (const ElementType& element) const;

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

    [[nodiscard]] auto& getNodes() { return nodes; }
    [[nodiscard]] const auto& getNodes() const { return nodes; }

    Node createEmptyNode()
    {
        return Node { nodeArena };
    }

protected:
    virtual ElementType& insertElementInternal (ElementType&& element, NodeVector& topLevelNodes) = 0;
    virtual void onDelete (const Node& /*nodeBeingDeleted*/) {}

    typename Node::NodeArena nodeArena;

private:
    void refreshElementIndexes();

    NodeVector nodes { nodeArena };
    int totalNumElements = 0;
};
} // namespace chowdsp

#include "chowdsp_AbstractTree.cpp"
