#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Chunk List Test", "[common][data-structures]")
{
    chowdsp::ArenaAllocator<> arena { 1024 };
    chowdsp::ChunkList<int, 10, chowdsp::ArenaAllocator<>> list { arena };
    REQUIRE (list.count() == 0);

    SECTION ("Usage (copy/for-each)")
    {
        static constexpr int N = 101;
        for (int i = 0; i < N; i++)
            list.insert (i);
        REQUIRE (list.count() == N);

        list.for_each ([i = 0] (int& item) mutable
                       {
            REQUIRE (item == i);
            item++;
            i++; });

        std::as_const (list).for_each ([i = 1] (int item) mutable
                                       {
            REQUIRE (item == i);
            i++; });

        list.clear();
        arena.clear();
        REQUIRE (list.count() == 0);
    }

    SECTION ("Usage (move/iterators)")
    {
        static constexpr int N = 101;
        for (int i = 0; i < N; i++)
        {
            auto ii = i;
            list.insert (std::move (ii));
        }
        REQUIRE (list.count() == N);

        for (auto [idx, item] : chowdsp::enumerate (list))
        {
            REQUIRE (item == static_cast<int> (idx));
            item++;
        }

        for (auto [idx, item] : chowdsp::enumerate (std::as_const (list)))
        {
            REQUIRE (item == static_cast<int> (idx + 1));
        }
    }

    SECTION ("Iterators")
    {
        for ([[maybe_unused]] auto _ : list)
            REQUIRE (false);
        REQUIRE ((list.begin() != list.end()) == false);

        {
            list.insert (4);
            auto iter = list.begin();
            REQUIRE (*iter == 4);
            ++iter;
            REQUIRE ((iter != list.end()) == false);
        }

        for (int i = 1; i < 11; i++)
            list.insert (i);

        auto iter = list.begin();
        auto end = list.end();
        REQUIRE (iter.chunk != end.chunk);
        REQUIRE (iter.chunk == &list.head_chunk);
        REQUIRE (end.chunk == list.tail_chunk);
        REQUIRE (end.index_in_chunk == 1);
    }
}
