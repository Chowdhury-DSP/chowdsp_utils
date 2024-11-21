#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Chunk List Test", "[common][data-structures]")
{
    chowdsp::ArenaAllocator<> arena { 1024 };
    chowdsp::ChunkList<int, 10, chowdsp::ArenaAllocator<>> list { arena };
    REQUIRE (list.count() == 0);

    static constexpr int N = 101;
    for (int i = 0; i < N; i++)
        list.insert (i);
    REQUIRE (list.count() == N);

    list.for_each ([i = 0] (int& item) mutable
    {
        REQUIRE (item == i);
        item++;
        i++;
    });

    std::as_const (list).for_each ([i = 1] (int item) mutable
    {
        REQUIRE (item == i);
        i++;
    });
}
