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

    static Node& insertOneElement (FakeData&& element, Node& parent, AbstractTree& tree)
    {
        auto* new_node = tree.createEmptyNode();
        new_node->leaf = std::move (element);
        new_node->prev_sibling = parent.last_child;
        if (parent.last_child != nullptr)
            parent.last_child->next_sibling = new_node;
        parent.last_child = new_node;
        return *new_node;
    }

    static Node& insertElementIntoNewSubtree (FakeData&& element,
                                              Node& parent,
                                              AbstractTree& tree,
                                              std::string_view tag)
    {
        auto* new_sub_tree_node = tree.createEmptyNode();
        new_sub_tree_node->tag = std::string { tag };
        new_sub_tree_node->prev_sibling = parent.last_child;
        if (parent.last_child != nullptr)
            parent.last_child->next_sibling = new_sub_tree_node;
        parent.last_child = new_sub_tree_node;
        return insertOneElement (std::move (element), *new_sub_tree_node, tree);
    }

    FakeData& insertElementInternal (FakeData&& element, Node& root) override
    {
        for (auto* iter = root.first_child; iter != nullptr; iter = iter->next_sibling)
        {
            if (iter->tag == positiveTag && element[0] > 0)
                return *insertOneElement (std::move (element), *iter, *this).leaf;

            if (iter->tag == negativeTag && element[0] < 0)
                return *insertOneElement (std::move (element), *iter, *this).leaf;

            if (iter->tag == zeroTag && element[0] == 0)
                return *insertOneElement (std::move (element), *iter, *this).leaf;
        }

        if (element[0] > 0)
            return *insertElementIntoNewSubtree (std::move (element), root, *this, positiveTag).leaf;

        if (element[0] < 0)
            return *insertElementIntoNewSubtree (std::move (element), root, *this, negativeTag).leaf;

        return *insertElementIntoNewSubtree (std::move (element), root, *this, zeroTag).leaf;
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
        int iter_count = 0;
        for (auto* iter = &tree.getRootNode(); iter != nullptr; iter = iter->next_sibling)
        {
            if (iter_count == 25)
            {
                (*iter->leaf)[0]++;
                benchmark::DoNotOptimize ((*iter->leaf)[0]);
                break;
            }
            iter_count++;
        }
    }
}
BENCHMARK (accessTree)->MinTime (0.5);

BENCHMARK_MAIN();
