#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

class ConstructCounts
{
    auto tie() const noexcept { return std::tie (constructions, copies, moves, calls, destructions); }

public:
    int constructions = 0;
    int copies = 0;
    int moves = 0;
    int calls = 0;
    int destructions = 0;

    ConstructCounts withConstructions (int i) const noexcept
    {
        auto c = *this;
        c.constructions = i;
        return c;
    }
    ConstructCounts withCopies (int i) const noexcept
    {
        auto c = *this;
        c.copies = i;
        return c;
    }
    ConstructCounts withMoves (int i) const noexcept
    {
        auto c = *this;
        c.moves = i;
        return c;
    }
    ConstructCounts withCalls (int i) const noexcept
    {
        auto c = *this;
        c.calls = i;
        return c;
    }
    ConstructCounts withDestructions (int i) const noexcept
    {
        auto c = *this;
        c.destructions = i;
        return c;
    }

    bool operator== (const ConstructCounts& other) const noexcept { return tie() == other.tie(); }
    bool operator!= (const ConstructCounts& other) const noexcept { return tie() != other.tie(); }
};

struct ConstructCounter
{
    explicit ConstructCounter (ConstructCounts& countsIn)
        : counts (countsIn) {}

    ConstructCounter (const ConstructCounter& c)
        : counts (c.counts)
    {
        counts.copies += 1;
    }

    ConstructCounter (ConstructCounter&& c) noexcept
        : counts (c.counts)
    {
        counts.moves += 1;
    }

    ~ConstructCounter() noexcept { counts.destructions += 1; }

    void operator()() const noexcept { counts.calls += 1; }

    ConstructCounts& counts;
};

static void toggleBool (bool& b) { b = ! b; }

TEST_CASE ("FixedSizeFunction Test", "[common][data-structures]")
{
    SECTION ("Can be constructed and called from a lambda")
    {
        const auto result = 5;
        bool wasCalled = false;
        const auto lambda = [&]
        { wasCalled = true; return result; };

        const chowdsp::FixedSizeFunction<sizeof (lambda), int()> fn (lambda);
        const auto out = fn();

        REQUIRE (wasCalled);
        REQUIRE (result == out);
    }

    SECTION ("void fn can be constructed from function with return value")
    {
        bool wasCalled = false;
        const auto lambda = [&]
        { wasCalled = true; return 5; };
        const chowdsp::FixedSizeFunction<sizeof (lambda), void()> fn (lambda);

        fn();
        REQUIRE (wasCalled);
    }

    SECTION ("Can be constructed and called from a function pointer")
    {
        bool state = false;

        const chowdsp::FixedSizeFunction<sizeof (void*), void (bool&)> fn (toggleBool);

        fn (state);
        REQUIRE (state);

        fn (state);
        REQUIRE (! state);

        fn (state);
        REQUIRE (state);
    }

    SECTION ("Default constructed functions throw if called")
    {
        const auto a = chowdsp::FixedSizeFunction<8, void()>();
        REQUIRE_THROWS_AS (a(), std::bad_function_call);

        const auto b = chowdsp::FixedSizeFunction<8, void()> (nullptr);
        REQUIRE_THROWS_AS (b(), std::bad_function_call);
    }

    SECTION ("Functions can be moved")
    {
        ConstructCounts counts;

        auto a = chowdsp::FixedSizeFunction<sizeof (ConstructCounter), void()> (ConstructCounter { counts });
        REQUIRE (counts == ConstructCounts().withMoves (1).withDestructions (1)); // The temporary gets destroyed

        a();
        REQUIRE (counts == ConstructCounts().withMoves (1).withDestructions (1).withCalls (1));

        const auto b = std::move (a);
        REQUIRE (counts == ConstructCounts().withMoves (2).withDestructions (1).withCalls (1));

        b();
        REQUIRE (counts == ConstructCounts().withMoves (2).withDestructions (1).withCalls (2));

        b();
        REQUIRE (counts == ConstructCounts().withMoves (2).withDestructions (1).withCalls (3));
    }

    SECTION ("Functions are destructed properly")
    {
        ConstructCounts counts;
        const ConstructCounter toCopy { counts };

        {
            auto a = chowdsp::FixedSizeFunction<sizeof (ConstructCounter), void()> (toCopy);
            REQUIRE (counts == ConstructCounts().withCopies (1));
        }

        REQUIRE (counts == ConstructCounts().withCopies (1).withDestructions (1));
    }

    SECTION ("Avoid destructing functions that fail to construct")
    {
        struct BadConstructor
        {
            explicit BadConstructor (ConstructCounts& c)
                : counts (c)
            {
                counts.constructions += 1;
                throw std::runtime_error { "this was meant to happen" };
            }

            BadConstructor (const BadConstructor&) = default;
            BadConstructor& operator= (const BadConstructor&) = delete;

            ~BadConstructor() noexcept { counts.destructions += 1; }

            void operator()() const noexcept { counts.calls += 1; }

            ConstructCounts& counts;
        };

        ConstructCounts counts;

        REQUIRE_THROWS_AS ((chowdsp::FixedSizeFunction<sizeof (BadConstructor), void()> (BadConstructor { counts })),
                           std::runtime_error);

        REQUIRE (counts == ConstructCounts().withConstructions (1));
    }

    SECTION ("Equality checks work")
    {
        chowdsp::FixedSizeFunction<8, void()> a;
        REQUIRE (! bool (a));
        REQUIRE (a == nullptr);
        REQUIRE (nullptr == a);
        REQUIRE (! (a != nullptr));
        REQUIRE (! (nullptr != a));

        chowdsp::FixedSizeFunction<8, void()> b ([] {});
        REQUIRE (bool (b));
        REQUIRE (b != nullptr);
        REQUIRE (nullptr != b);
        REQUIRE (! (b == nullptr));
        REQUIRE (! (nullptr == b));
    }

    SECTION ("Functions can be cleared")
    {
        chowdsp::FixedSizeFunction<8, void()> fn ([] {});
        REQUIRE (bool (fn));

        fn = nullptr;
        REQUIRE (! bool (fn));
    }

    SECTION ("Functions can be assigned")
    {
        using Fn = chowdsp::FixedSizeFunction<8, void()>;

        int numCallsA = 0;
        int numCallsB = 0;

        Fn x;
        Fn y;
        REQUIRE (! bool (x));
        REQUIRE (! bool (y));

        x = [&]
        { numCallsA += 1; };
        y = [&]
        { numCallsB += 1; };
        REQUIRE (bool (x));
        REQUIRE (bool (y));

        x();
        REQUIRE (numCallsA == 1);
        REQUIRE (numCallsB == 0);

        y();
        REQUIRE (numCallsA == 1);
        REQUIRE (numCallsB == 1);

        x = std::move (y);
        REQUIRE (numCallsA == 1);
        REQUIRE (numCallsB == 1);

        x();
        REQUIRE (numCallsA == 1);
        REQUIRE (numCallsB == 2);
    }

    SECTION ("Functions may mutate internal state")
    {
        using Fn = chowdsp::FixedSizeFunction<64, void()>;

        Fn x;
        REQUIRE (! bool (x));

        int numCalls = 0;
        x = [&numCalls, counter = 0]() mutable
        { counter += 1; numCalls = counter; };
        REQUIRE (bool (x));

        REQUIRE (numCalls == 0);

        x();
        REQUIRE (numCalls == 1);

        x();
        REQUIRE (numCalls == 2);
    }

    SECTION ("Functions can sink move-only parameters")
    {
        using FnA = chowdsp::FixedSizeFunction<64, int (std::unique_ptr<int>)>;

        auto value = 5;
        auto ptr = std::make_unique<int> (value);

        FnA fnA = [] (std::unique_ptr<int> p)
        { return *p; };

        REQUIRE (value == fnA (std::move (ptr)));

        using FnB = chowdsp::FixedSizeFunction<64, void (std::unique_ptr<int> &&)>;

        FnB fnB = [&value] (std::unique_ptr<int>&& p)
        {
            auto x = std::move (p);
            value = *x;
        };

        const auto newValue = 10;
        fnB (std::make_unique<int> (newValue));
        REQUIRE (value == newValue);
    }

    SECTION ("Functions be converted from smaller functions")
    {
        using SmallFn = chowdsp::FixedSizeFunction<20, void()>;
        using LargeFn = chowdsp::FixedSizeFunction<21, void()>;

        bool smallCalled = false;
        bool largeCalled = false;

        SmallFn small = [&smallCalled, a = std::array<char, 8> {}]
        { smallCalled = true; juce::ignoreUnused (a); };
        LargeFn large = [&largeCalled, a = std::array<char, 8> {}]
        { largeCalled = true; juce::ignoreUnused (a); };

        large = std::move (small);

        large();

        REQUIRE (smallCalled);
        REQUIRE (! largeCalled);
    }
}
