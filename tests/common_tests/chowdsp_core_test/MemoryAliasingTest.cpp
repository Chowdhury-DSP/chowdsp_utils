#include <TimedUnitTest.h>
#include <juce_events/juce_events.h>
#include <chowdsp_core/chowdsp_core.h>

using namespace chowdsp::MemoryUtils;

class MemoryAliasingTest : public TimedUnitTest
{
public:
    MemoryAliasingTest() : TimedUnitTest ("Memory Aliasing Test") {}

    void rawMemoryTest()
    {
        int x[10];
        expect (doesMemoryAlias (x, 10, x, 5));
        expect (doesMemoryAlias (x, 10, x + 5, 5));
        expect (! doesMemoryAlias (x, 5, x + 5, 5));
    }

    void cStyleArrayTest()
    {
        int x[10];
        int y[20];
        expect (doesMemoryAlias (x, x));
        expect (! doesMemoryAlias (x, y));
    }

    void stdArrayTest()
    {
        std::array<int, 10> x;
        std::array<int, 20> y;
        expect (doesMemoryAlias (x, x));
        expect (! doesMemoryAlias (x, y));
    }

    void stdVectorTest()
    {
        std::vector<int> x { 0, 1, 2, 3 };
        std::vector<int> y { 0, 1, 2 };
        expect (doesMemoryAlias (x, x));
        expect (! doesMemoryAlias (x, y));
    }

    void iteratorsTest()
    {
        std::vector<int> x { 0, 1, 2, 3 };
        std::vector<int> y { 0, 1, 2 };
        expect (doesMemoryAlias (x.begin(), x.end(), x.begin(), x.end()));
        expect (! doesMemoryAlias (x.begin(), x.end(), y.begin(), y.end()));
        expect (doesMemoryAlias (x.begin(), x.begin() + 2, x.begin(), x.end()));
        expect (! doesMemoryAlias (x.begin(), x.begin() + 2, x.begin() + 2, x.end()));
    }

    void runTestTimed() override
    {
        beginTest ("Raw Memory Test");
        rawMemoryTest();

        beginTest ("C-Style Array Test");
        cStyleArrayTest();

        beginTest ("std::array Test");
        stdArrayTest();

        beginTest ("std::vector Test");
        stdVectorTest();

        beginTest ("Iterators Test");
        iteratorsTest();
    }
};

static MemoryAliasingTest memoryAliasingTest;
