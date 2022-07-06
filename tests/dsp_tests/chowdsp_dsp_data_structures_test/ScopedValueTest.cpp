#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

TEST_CASE ("Scoped Value Test")
{
    SECTION ("Read/Write Test")
    {
        constexpr float testVal1 = 2.0f;
        float x = 0.0f;
        {
            chowdsp::ScopedValue x_scoped { x };
            REQUIRE_MESSAGE (x_scoped.get() == x, "Initial value is incorrect!");

            x_scoped.get() = testVal1;
            REQUIRE_MESSAGE (x_scoped.get() == testVal1, "Set value is incorrect!");
        }

        REQUIRE_MESSAGE (x == testVal1, "Value after scope is incorrect!");
    }
}
