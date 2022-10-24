#include <CatchUtils.h>
#include <chowdsp_filters/chowdsp_filters.h>

TEST_CASE ("Linear Transforms Test")
{
    SECTION ("Negative Feedback Test")
    {
        { // without normalization
            float b[2] = { 0.5f, -1.0f };
            float a[2] = { 0.5f, 1.0f };
            chowdsp::LinearTransforms::transformFeedback<1, float, false> (b, a, 0.5f);

            REQUIRE (b[0] == 0.5f);
            REQUIRE (b[1] == -1.0f);
            REQUIRE (a[0] == 0.25f);
            REQUIRE (a[1] == 1.5f);
        }

        { // with normalization
            double b[2] = { 0.5, -1.0 };
            double a[2] = { 0.5, 1.0 };
            chowdsp::LinearTransforms::transformFeedback<1> (b, a, 0.5);

            REQUIRE (b[0] == 2.0);
            REQUIRE (b[1] == -4.0);
            REQUIRE (a[0] == 1.0);
            REQUIRE (a[1] == 6.0);
        }
    }
}
