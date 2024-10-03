#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

struct StringTree : chowdsp::AbstractTree<std::string, StringTree>
{
    using AbstractTree::AbstractTree;

    static Node& insert_string (std::string&& element, Node& parent_node, AbstractTree& tree)
    {
        auto* new_node = tree.createLeafNode (std::move (element));

        insertNodeSorted (parent_node, new_node, [] (const Node& el1, const Node& el2)
                          { return el1.value.leaf() < el2.value.leaf(); });
        return *new_node;
    }

    static std::string& insertElementInternal (StringTree& self, std::string&& element, Node& root)
    {
        for (auto* iter = root.first_child; iter != nullptr; iter = iter->next_sibling)
        {
            if (iter->value.tag() == std::string_view { element.data(), 1 })
                return insert_string (std::move (element), *iter, self).value.leaf();
        }

        auto* new_sub_tree = self.createTagNode ({ element.data(), 1 });
        insertNodeSorted (root, new_sub_tree, [] (const Node& el1, const Node& el2)
                          { return el1.value.tag() < el2.value.tag(); });
        return insert_string (std::move (element), *new_sub_tree, self).value.leaf();
    }

    auto& get_allocator()
    {
        return allocator;
    }
};

TEST_CASE ("Abstract Tree Test", "[common][data-structures]")
{
    const std::vector<std::string> foods { "alfalfa", "apples", "beets", "donuts" };

    StringTree tree { 7 };
    tree.insertElements (std::vector { foods });
    REQUIRE (tree.size() == 4);

    SECTION ("Clear")
    {
        tree.clear();
        REQUIRE (tree.size() == 0);
    }

    SECTION ("Insertion")
    {
        REQUIRE (tree.get_allocator().get_arena_count() == 1);
        tree.reserve (6); // should jassert!
        REQUIRE (tree.get_allocator().get_arena_count() == 1);

        tree.insertElement ("almonds");
        REQUIRE (tree.size() == 5);

        {
            const auto* a_node = tree.getRootNode().first_child;
            REQUIRE (a_node->value.tag() == "a");
            REQUIRE (a_node->first_child->value.leaf() == "alfalfa");
            REQUIRE (a_node->first_child->next_sibling->value.leaf() == "almonds");
            REQUIRE (a_node->first_child->next_sibling->next_sibling->value.leaf() == "apples");
        }

        tree.insertElement ("acai");
        REQUIRE (tree.size() == 6);

        {
            const auto* a_node = tree.getRootNode().first_child;
            REQUIRE (a_node->value.tag() == "a");
            REQUIRE (a_node->first_child->value.leaf() == "acai");
            REQUIRE (a_node->first_child->next_sibling->value.leaf() == "alfalfa");
            REQUIRE (a_node->first_child->next_sibling->next_sibling->value.leaf() == "almonds");
            REQUIRE (a_node->first_child->next_sibling->next_sibling->next_sibling->value.leaf() == "apples");
        }

        REQUIRE (tree.get_allocator().get_arena_count() == 2);
        tree.shrinkArena(); // should jassert!
        REQUIRE (tree.get_allocator().get_arena_count() == 2);
        tree.clear();
        REQUIRE (tree.get_allocator().get_arena_count() == 2);
        tree.shrinkArena();
        REQUIRE (tree.get_allocator().get_arena_count() == 1);
    }

    SECTION ("Remove One")
    {
        tree.removeElement ("beets");
        REQUIRE (tree.size() == 3);

        const auto* d_node = tree.getRootNode().first_child->next_sibling;
        REQUIRE (d_node->value.tag() == "d");
    }

    SECTION ("Remove From Start of Sub-Tree")
    {
        tree.removeElement ("alfalfa");
        REQUIRE (tree.size() == 3);

        const auto* a_node = tree.getRootNode().first_child;
        REQUIRE (a_node->first_child->value.leaf() == "apples");
    }

    SECTION ("Remove From End of Sub-Tree")
    {
        tree.removeElement ("apples");
        REQUIRE (tree.size() == 3);

        const auto* a_node = tree.getRootNode().first_child;
        REQUIRE (a_node->first_child->value.leaf() == "alfalfa");
    }

    SECTION ("Remove Last Node in Sub-Tree")
    {
        tree.removeElement ("donuts");
        REQUIRE (tree.size() == 3);

        const auto* b_node = tree.getRootNode().first_child->next_sibling;
        REQUIRE (b_node->value.tag() == "b");
    }

    SECTION ("Remove Multiple")
    {
        tree.removeElements ([] (const std::string& el)
                             { return el.find ('t') != std::string::npos; });
        REQUIRE (tree.size() == 2);

        REQUIRE (tree.getRootNode().first_child->next_sibling == nullptr);
        REQUIRE (tree.getRootNode().first_child->prev_sibling == nullptr);
        REQUIRE (tree.getRootNode().first_child->value.tag() == "a");
    }

    SECTION ("Remove All")
    {
        tree.removeElements ([] (const std::string& el)
                             { return ! el.empty(); });
        REQUIRE (tree.size() == 0);

        tree.insertElement ("mussels");
        REQUIRE (tree.size() == 1);
        REQUIRE (tree.getRootNode().first_child->value.tag() == "m");
        REQUIRE (tree.getRootNode().first_child->first_child->value.leaf() == "mussels");

        auto found = tree.findElement ("mussels");
        REQUIRE (found);
    }

    SECTION ("Find Success")
    {
        auto found = std::as_const (tree).findElement ("apples");
        jassert (found);
    }

    SECTION ("Find Fail")
    {
        [[maybe_unused]] auto found = std::as_const (tree).findElement ("bologna");
        jassert (! found);
    }

    SECTION ("To Uppercase")
    {
        tree.doForAllElements (
            [] (std::string& str)
            {
                for (auto& c : str)
                    c = static_cast<char> (std::toupper (static_cast<int> (c)));
            });

        std::as_const (tree).doForAllElements (
            [] (const std::string& str)
            {
                for (auto& c : str)
                    REQUIRE (((c >= 'A') && (c <= 'Z')));
            });
    }
}
