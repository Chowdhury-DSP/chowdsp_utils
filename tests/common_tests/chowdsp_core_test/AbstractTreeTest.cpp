#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>

struct StringTree : chowdsp::AbstractTree<std::string>
{
    static Node& insertString (std::string&& element, std::vector<Node>& nodes)
    {
        Node newNode {};
        newNode.leaf = std::move (element);
        return *chowdsp::VectorHelpers::insert_sorted (nodes,
                                                       std::move (newNode),
                                                       [] (const Node& el1, const Node& el2)
                                                       {
                                                           return *el1.leaf < *el2.leaf;
                                                       });
    }

    std::string& insertElementInternal (std::string&& element, std::vector<Node>& topLevelNodes) override
    {
        for (auto& node : topLevelNodes)
        {
            if (node.tag == std::string { element[0] })
                return *insertString (std::move (element), node.subtree).leaf;
        }

        Node newSubTreeNode {};
        newSubTreeNode.tag = std::string { element[0] };
        auto& insertedSubTreeNode = *chowdsp::VectorHelpers::insert_sorted (topLevelNodes,
                                                                            std::move (newSubTreeNode),
                                                                            [] (const Node& el1, const Node& el2)
                                                                            { return el1.tag < el2.tag; });
        return *insertString (std::move (element), insertedSubTreeNode.subtree).leaf;
    }
};

TEST_CASE ("Abstract Tree Test", "[common][data-structures]")
{
    const std::vector<std::string> foods { "alfalfa", "apples", "beets", "donuts" };

    StringTree tree;
    tree.insertElements (std::vector { foods });
    REQUIRE (tree.size() == 4);

    SECTION ("Insertion")
    {
        {
            size_t counter = 0;
            std::as_const (tree).doForAllElements ([&foods, &counter] (const std::string& str)
                                                   { REQUIRE (foods[counter++] == str); });
        }

        tree.insertElement ("almonds");
        REQUIRE (tree.getNodes()[0].subtree.size() == 3);

        tree.clear();
        REQUIRE (tree.size() == 0);
    }
}
