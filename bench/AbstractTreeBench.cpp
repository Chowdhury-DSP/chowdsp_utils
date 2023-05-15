#include <benchmark/benchmark.h>
#include <list>
#include <iostream>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

using FakeData = std::array<int32_t, 10>;

struct DataTree : chowdsp::AbstractTree<FakeData>
{
    static constexpr std::string_view positiveTag = "positive";
    static constexpr std::string_view negativeTag = "negative";
    static constexpr std::string_view zeroTag = "zero";

    static Node& insertOneElement (FakeData&& element, NodeVector& nodes, chowdsp::AbstractTree<FakeData>& tree)
    {
        auto newNode = tree.createEmptyNode();
        newNode.leaf = std::move (element);
        return *nodes.insert (nodes.end(), std::move (newNode));
    }

    static Node& insertElementIntoNewSubtree (FakeData&& element,
                                              NodeVector& topLevelNodes,
                                              chowdsp::AbstractTree<FakeData>& tree,
                                              std::string_view tag)
    {
        auto newSubTreeNode = tree.createEmptyNode();
        newSubTreeNode.tag = std::string { tag };
        auto& insertedSubTreeNode = *topLevelNodes.insert (topLevelNodes.end(), std::move (newSubTreeNode));
        return insertOneElement (std::move (element), insertedSubTreeNode.subtree, tree);
    }

    FakeData& insertElementInternal (FakeData&& element, NodeVector& topLevelNodes) override
    {
        for (auto& node : topLevelNodes)
        {
            if (node.tag == positiveTag && element[0] > 0)
                return *insertOneElement (std::move (element), node.subtree, *this).leaf;

            if (node.tag == negativeTag && element[0] < 0)
                return *insertOneElement (std::move (element), node.subtree, *this).leaf;

            if (node.tag == zeroTag && element[0] == 0)
                return *insertOneElement (std::move (element), node.subtree, *this).leaf;
        }

        if (element[0] > 0)
            return *insertElementIntoNewSubtree (std::move (element), topLevelNodes, *this, positiveTag).leaf;

        if (element[0] < 0)
            return *insertElementIntoNewSubtree (std::move (element), topLevelNodes, *this, negativeTag).leaf;

        return *insertElementIntoNewSubtree (std::move (element), topLevelNodes, *this, zeroTag).leaf;
    }
};

static void insertVector (benchmark::State& state)
{
    std::vector<FakeData> vec;
    for (auto _ : state)
    {
        for (int32_t i = -10; i <= 10; ++i)
        {
            FakeData data {};
            std::fill (std::begin (data), std::end (data), i);
            vec.emplace_back (std::move (data));
        }
        vec.clear();
    }
}
BENCHMARK (insertVector)->MinTime (0.5);

static void insertList (benchmark::State& state)
{
    std::list<FakeData> list {};
    for (auto _ : state)
    {
        for (int32_t i = -10; i <= 10; ++i)
        {
            FakeData data {};
            std::fill (std::begin (data), std::end (data), i);
            list.emplace_back (std::move (data));
        }
        list.clear();
    }
}
BENCHMARK (insertList)->MinTime (0.5);

static void insertTree (benchmark::State& state)
{
    DataTree tree {};
    for (auto _ : state)
    {
        for (int32_t i = -10; i <= 10; ++i)
        {
            FakeData data {};
            std::fill (std::begin (data), std::end (data), i);
            tree.insertElement (std::move (data));
        }
        tree.clear();
    }
}
BENCHMARK (insertTree)->MinTime (0.5);

static void insertTreeMany (benchmark::State& state)
{
    DataTree tree {};
    for (auto _ : state)
    {
        std::vector<FakeData> vec;
        vec.reserve (21);
        for (int32_t i = -10; i <= 10; ++i)
        {
            FakeData data {};
            std::fill (std::begin (data), std::end (data), i);
            vec.emplace_back (std::move (data));
        }
        tree.insertElements (std::move (vec));
        tree.clear();
    }
}
BENCHMARK (insertTreeMany)->MinTime (0.5);

static void iterateVector (benchmark::State& state)
{
    std::vector<FakeData> vec;
    for (int32_t i = -30; i <= 30; ++i)
    {
        FakeData data {};
        std::fill (std::begin (data), std::end (data), i);
        vec.emplace_back (std::move (data));
    }

    for (auto _ : state)
    {
        for (auto& data : vec)
            data[0]++;
    }
}
BENCHMARK (iterateVector)->MinTime (0.5);

static void iterateList (benchmark::State& state)
{
    std::list<FakeData> list;
    for (int32_t i = -30; i <= 30; ++i)
    {
        FakeData data {};
        std::fill (std::begin (data), std::end (data), i);
        list.emplace_back (std::move (data));
    }

    for (auto _ : state)
    {
        for (auto& data : list)
            data[0]++;
    }
}
BENCHMARK (iterateList)->MinTime (0.5);

static void iterateTree (benchmark::State& state)
{
    std::vector<FakeData> vec;
    for (int32_t i = -30; i <= 30; ++i)
    {
        FakeData data {};
        std::fill (std::begin (data), std::end (data), i);
        vec.emplace_back (std::move (data));
    }

    DataTree tree;
    tree.insertElements (std::move (vec));

    for (auto _ : state)
    {
        tree.doForAllElements ([] (FakeData& data)
                               { data[0]++; });
    }
}
BENCHMARK (iterateTree)->MinTime (0.5);

static void accessVector (benchmark::State& state)
{
    std::vector<FakeData> vec;
    for (int32_t i = -30; i <= 30; ++i)
    {
        FakeData data {};
        std::fill (std::begin (data), std::end (data), i);
        vec.emplace_back (std::move (data));
    }

    for (auto _ : state)
    {
        vec[25][0]++;
        benchmark::DoNotOptimize (vec[25][0]);
    }
}
BENCHMARK (accessVector)->MinTime (0.5);

static void accessList (benchmark::State& state)
{
    std::list<FakeData> list;
    for (int32_t i = -30; i <= 30; ++i)
    {
        FakeData data {};
        std::fill (std::begin (data), std::end (data), i);
        list.emplace_back (std::move (data));
    }

    for (auto _ : state)
    {
        auto iter = list.begin();
        std::advance (iter, 25);
        (*iter)[0]++;
        benchmark::DoNotOptimize (*iter);
    }
}
BENCHMARK (accessList)->MinTime (0.5);

static void accessTree (benchmark::State& state)
{
    std::vector<FakeData> vec;
    for (int32_t i = -30; i <= 30; ++i)
    {
        FakeData data {};
        std::fill (std::begin (data), std::end (data), i);
        vec.emplace_back (std::move (data));
    }

    DataTree tree;
    tree.insertElements (std::move (vec));

    for (auto _ : state)
    {
        (*tree.getElementByIndex (25))[0]++;
    }
}
BENCHMARK (accessTree)->MinTime (0.5);

BENCHMARK_MAIN();
