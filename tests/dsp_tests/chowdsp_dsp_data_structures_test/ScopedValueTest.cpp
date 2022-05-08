#include <TimedUnitTest.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

class ScopedValueTest : public TimedUnitTest
{
public:
    ScopedValueTest() : TimedUnitTest ("Scoped Value Test")
    {
    }

    void readWriteTest()
    {
        constexpr float testVal1 = 2.0f;
        float x = 0.0f;
        {
            chowdsp::ScopedValue x_scoped { x };
            expectEquals (x_scoped.get(), x, "Initial value is incorrect!");

            x_scoped.get() = testVal1;
            expectEquals (x_scoped.get(), testVal1, "Set value is incorrect!");
        }

        expectEquals (x, testVal1, "Value after scope is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("Read/Write Test");
        readWriteTest();
    }
};

static ScopedValueTest scopedValueTest;
