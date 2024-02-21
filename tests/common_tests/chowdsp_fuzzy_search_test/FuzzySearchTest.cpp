#include <chowdsp_fuzzy_search/chowdsp_fuzzy_search.h>

#include <CatchUtils.h>

TEST_CASE ("Database Word Storage Test", "[common][search]")
{
    chowdsp::search_database::WordStorage wordStorage;

    REQUIRE (wordStorage.addWord ("test") == 0);
    REQUIRE (wordStorage.addWord ("test") == 0);
    REQUIRE (wordStorage.addWord ("blah") == 1);
    REQUIRE (wordStorage.addWord ("test") == 0);
    REQUIRE (wordStorage.addWord ("fork") == 2);

    REQUIRE (wordStorage.getString (wordStorage.wordViewList[0]) == "test");
    REQUIRE (wordStorage.getString (wordStorage.wordViewList[1]) == "blah");
    REQUIRE (wordStorage.getString (wordStorage.wordViewList[2]) == "fork");

    REQUIRE (wordStorage.getWordCount() == 3);
}

TEST_CASE ("String Split Test", "[common][search]")
{
    chowdsp::ArenaAllocator<> arena;
    arena.reset (1024);

    {
        const auto _ = arena.create_frame();
        const auto splits = chowdsp::search_helpers::splitString ("blah", arena);
        REQUIRE (splits.size() == 1);
        REQUIRE (splits[0] == "blah");
    }

    {
        const auto _ = arena.create_frame();
        const auto splits = chowdsp::search_helpers::splitString ("blah blah", arena);
        REQUIRE (splits.size() == 2);
        REQUIRE (splits[0] == "blah");
        REQUIRE (splits[1] == "blah");
    }

    {
        const auto _ = arena.create_frame();
        const auto splits = chowdsp::search_helpers::splitString ("This is, a sentence", arena);
        REQUIRE (splits.size() == 4);
        REQUIRE (splits[0] == "This");
        REQUIRE (splits[1] == "is");
        REQUIRE (splits[2] == "a");
        REQUIRE (splits[3] == "sentence");
    }
}

TEST_CASE ("Basic Search Test", "[common][search]")
{
    // pretending we're searching for books by fields:
    // - name
    // - author
    // - genre
    // - publisher

    struct BookInfo
    {
        std::string_view name;
        std::string_view author;
        std::string_view genre;
        std::string_view publisher;
    };

    static constexpr std::array books {
        BookInfo { "The Hobbit", "J.R.R. Tolkien", "Fantasy", "Houghton Mifflin" },
        BookInfo { "Harry Potter", "J.K. Rowling", "Fantasy", "Scholastic" },
        BookInfo { "A Brief History of Time", "Stephen Hawking", "Science", "Bantam Books" },
        BookInfo { "Willy Wonka", "Roald Dahl", "Fantasy", "Penguin Books" },
        BookInfo { "The Grapes of Wrath", "John Steinbeck", "Fiction", "Viking Press" },
        BookInfo { "If On a Winter's Night a Traveler", "Italo Calvino", "Fiction", "Harcourt" },
        BookInfo { "Main Street", "Sinclair Lewis", "Fiction", "Harcourt" },
        BookInfo { "Electromagnetic Theory", "Oliver Heaviside", "Science", "Dover Publications" },
    };

    chowdsp::SearchDatabase<size_t, 4> bookSearcher;

    bookSearcher.resetEntries (books.size());
    for (const auto [idx, book] : chowdsp::enumerate (books))
        bookSearcher.addEntry (idx, { book.name, book.author, book.genre, book.publisher });

    bookSearcher.setWeights ({ 1.0f, 1.0f, 0.5f, 0.3f });
    bookSearcher.prepareForSearch();

    {
        const auto results = bookSearcher.search ("Stei");
        REQUIRE (books[results[0].key].name == "The Grapes of Wrath");
    }

    {
        const auto results = bookSearcher.search ("Harcourt");
        REQUIRE (results.size() == 2);
    }

    {
        const auto results = bookSearcher.search ("Fiction");
        REQUIRE (results.size() == 3);
    }
}

#if ! JUCE_DEBUG
#include "TestData.h"

TEST_CASE ("Test With Large Database", "[common][search]")
{
    chowdsp::SearchDatabase<size_t, 5> f;
    f.resetEntries (std::size (entries), 100'000);

    const auto t1 = std::chrono::steady_clock::now();
    for (const auto [idx, e] : chowdsp::enumerate (entries))
        f.addEntry (idx, { e.brand, e.modslug, e.modname, e.moddesc, e.tags });

    const auto t2 = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> fp_ms = t2 - t1;

    REQUIRE (f.countEntries() == std::size (entries));

    std::cout << f.countEntries() << std::endl;
    std::cout << f.wordStorage.wordViewList.size() << std::endl;
    std::cout << "FillDB Time: " << fp_ms.count() << " ms\n";
}
#endif